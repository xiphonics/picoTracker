/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "MidiInstrument.h"
#include "Application/Model/Scale.h"
#include "Application/Player/Player.h"
#include "Application/Utils/char.h"
#include "CommandList.h"
#include "Externals/etl/include/etl/to_string.h"
#include "Services/Midi/MidiMessage.h"
#include "System/Console/Trace.h"

MidiService *MidiInstrument::svc_ = 0;
TimerService *MidiInstrument::timerSvc_ = 0;
MidiInstrument::NoteOffInfo MidiInstrument::NoteOffInfo::current = {0, 0};

MidiInstrument::MidiInstrument()
    : I_Instrument(&variables_), channel_(FourCC::MidiInstrumentChannel, 0),
      noteLen_(FourCC::MidiInstrumentNoteLength, 0),
      volume_(FourCC::MidiInstrumentVolume, 255),
      table_(FourCC::MidiInstrumentTable, VAR_OFF),
      tableAuto_(FourCC::MidiInstrumentTableAutomation, false),
      program_(FourCC::MidiInstrumentProgram, VAR_OFF) {

  if (svc_ == 0) {
    svc_ = MidiService::GetInstance();
  };

  if (timerSvc_ == 0) {
    timerSvc_ = TimerService::GetInstance();
  };

  // name_ is now an etl::string in the base class, not a Variable
  variables_.insert(variables_.end(), &channel_);
  variables_.insert(variables_.end(), &noteLen_);
  variables_.insert(variables_.end(), &volume_);
  variables_.insert(variables_.end(), &table_);
  variables_.insert(variables_.end(), &tableAuto_);
  variables_.insert(variables_.end(), &program_);
}

MidiInstrument::~MidiInstrument(){};

bool MidiInstrument::Init() {
  tableState_.Reset();
  return true;
};

void MidiInstrument::OnStart() {
  tableState_.Reset();

  // Send program change message at the start of playback
  int program = program_.GetInt();

  // Only send program change if a valid program is set
  // 0x80 is used to indicate "OFF"
  if (program != VAR_OFF && program >= 0 && program <= 0x7F) {
    SendProgramChange(channel_.GetInt(), program);
  }

  MidiMessage msg;
  // send instrument volume for this midi channel when it's not zero
  int volume = volume_.GetInt();
  if (volume > 0) {
    msg.status_ = MidiMessage::MIDI_CONTROL_CHANGE + channel_.GetInt();
    msg.data1_ = MidiCC::CC_VOLUME;
    msg.data2_ = volume / 2;
    svc_->QueueMessage(msg);
  }

  svc_->RegisterActiveChannel(channel_.GetInt());
};

bool MidiInstrument::Start(int c, unsigned char note, bool retrigger) {

  first_[c] = true;
  lastNotes_[c][0] = note;

  Variable *v = FindVariable(FourCC::MidiInstrumentChannel);
  int channel = v->GetInt();

  v = FindVariable(FourCC::MidiInstrumentNoteLength);
  remainingTicks_ = v->GetInt();
  if (remainingTicks_ == 0) {
    remainingTicks_ = -1;
  }

  // set initial velocity (changed via InstrumentCommandVelocity)
  velocity_ = INITIAL_NOTE_VELOCITY;
  playing_ = true;
  retrig_ = false;
  pitchBend_ = false;
  useLogCurve_ = false;

  return true;
};

void MidiInstrument::Stop(int c) {

  Trace::Debug("MIDI INSTR STOP!====");

  Variable *v = FindVariable(FourCC::MidiInstrumentChannel);
  int channel = v->GetInt();

  for (int i = 0; i < MAX_MIDI_CHORD_NOTES + 1; i++) {
    if (lastNotes_[c][i] == 0) {
      continue;
    }
    MidiMessage msg;
    msg.status_ = MidiMessage::MIDI_NOTE_OFF + channel;
    msg.data1_ = lastNotes_[c][i];
    msg.data2_ = 0x00;
    svc_->QueueMessage(msg);
    Trace::Debug("MIDI chord note OFF[%d]:%d", i, msg.data1_);
  }
  // clear last notes array
  lastNotes_[c].fill(0);
  playing_ = false;
};

void MidiInstrument::SetChannel(int channel) {
  Variable *v = FindVariable(FourCC::MidiInstrumentChannel);
  v->SetInt(channel);
};

bool MidiInstrument::Render(int channel, fixed *buffer, int size,
                            bool updateTick) {

  // We do it here so we have the opportunity to send some command before
  Variable *v = FindVariable(FourCC::MidiInstrumentChannel);
  int mchannel = v->GetInt();
  if (first_[channel]) {
    // send note
    MidiMessage msg;
    msg.status_ = MidiMessage::MIDI_NOTE_ON + mchannel;
    msg.data1_ = lastNotes_[channel][0];
    msg.data2_ = velocity_;
    svc_->QueueMessage(msg);

    first_[channel] = false;
  }

  // Update pitch bend logic if a pitch bend is active.
  if (updateTick) {
    if (pitchBend_) {
      int8_t prev = pitchBendCurrent_;
      if (pitchBendSpeed_ == 0) {
        pitchBendCurrent_ = pitchBendTarget_;
      } else {
        // Calculate the difference and sign for pitch bend direction.
        float diff = pitchBendTarget_ - pitchBendCurrent_;
        float sign = (diff > 0) ? 1.0f : -1.0f;
        float nextValue = pitchBendCurrent_ + (sign * pitchBendStep_);
        if ((sign > 0 && nextValue >= pitchBendTarget_) ||
            (sign < 0 && nextValue <= pitchBendTarget_)) {
          pitchBendCurrent_ = pitchBendTarget_;
          pitchBend_ = false;
          pitchBendStep_ = 1.0f;
        } else {
          if (useLogCurve_) {
            // Exponential pitch bend calculation.
            float diff = pitchBendTarget_ - pitchBendCurrent_;
            pitchBendCurrent_ += diff * interpolationAlpha_;
            // If the pitch is close enough to the target, snap to target and
            // stop bending.
            if (fabs(diff) < PB_MAX_ALPHA) {
              pitchBendCurrent_ = pitchBendTarget_;
              pitchBend_ = false;
            }
          } else {
            // Linear pitch bend calculation.
            pitchBendStep_ = (diff > 0 ? 1 : -1) *
                             (abs(diff) / static_cast<float>(pitchBendSpeed_));
            pitchBendCurrent_ += pitchBendStep_;
          }
        }
      }
      // If pitch bend value changed, send MIDI pitch bend message.
      if (pitchBendCurrent_ != prev) {
        // Convert internal pitch bend value to MIDI pitch bend range (0-16383).
        int16_t midiValue =
            ((pitchBendCurrent_ - PB_7BIT_MAX) * PB_CENTER) / PB_7BIT_MAX;
        int16_t bend = midiValue + PB_CENTER;
        // Clamp bend value to valid MIDI range.
        if (bend < 0) {
          bend = 0;
        } else if (bend > PB_MAX) {
          bend = PB_MAX;
        }
        // Create and queue MIDI pitch bend message.
        MidiMessage msg;
        msg.status_ = MidiMessage::MIDI_PITCH_BEND + mchannel;
        msg.data1_ = bend & 0x7F;
        msg.data2_ = (bend >> 7) & 0x7F;
        svc_->QueueMessage(msg);
      }
    }
  }

  if (remainingTicks_ > 0) {
    remainingTicks_--;
    if (remainingTicks_ == 0) {
      if (!retrig_) {
        Stop(channel);
      } else {
        MidiMessage msg;
        remainingTicks_ = retrigLoop_;
        msg.status_ = MidiMessage::MIDI_NOTE_OFF + mchannel;
        msg.data1_ = lastNotes_[channel][0];
        msg.data2_ = 0x00;
        svc_->QueueMessage(msg);
        msg.status_ = MidiMessage::MIDI_NOTE_ON + mchannel;
        msg.data1_ = lastNotes_[channel][0];
        msg.data2_ = velocity_;
        svc_->QueueMessage(msg);
      };
    };
  };
  return false;
};

bool MidiInstrument::IsInitialized() {
  return true; // Always initialised
};

void MidiInstrument::ProcessCommand(int channel, FourCC cc, ushort value) {

  Variable *v = FindVariable(FourCC::MidiInstrumentChannel);
  int mchannel = v->GetInt();

  switch (cc) {

  case FourCC::InstrumentCommandRetrigger: {
    unsigned char loop = (value & 0xFF); // number of ticks before repeat
    if (loop != 0) {
      retrig_ = true;
      retrigLoop_ = loop;
      remainingTicks_ = loop;
    } else {
      retrig_ = false;
    }
  } break;

  case FourCC::InstrumentCommandLegato: {
    pitchBendTarget_ = uint8_t(value & 0xFF);
    pitchBendSpeed_ = uint8_t(value >> 8);
    pitchBend_ = true;
    useLogCurve_ = true;

    // Convert to interpolation alpha using a nonlinear curve.
    float growthFactor =
        PB_MIN_GROWTH_FACTOR + (PB_MAX_GROWTH_FACTOR - PB_MIN_GROWTH_FACTOR) *
                                   (1.0f - ((pitchBendSpeed_ - 1) / 253.0f));
    float normalized = (growthFactor - 1.0f) / (PB_MAX_GROWTH_FACTOR - 1.0f);
    interpolationAlpha_ = powf(normalized, PB_CURVE_SHAPE) * PB_MAX_ALPHA;
  } break;

  case FourCC::InstrumentCommandPitchSlide: {
    pitchBendTarget_ = uint8_t(value & 0xFF);
    pitchBendSpeed_ = uint8_t(value >> 8);
    pitchBend_ = true;
    pitchBendStep_ = 1.0f;
    useLogCurve_ = false;
  } break;

  case FourCC::InstrumentCommandVelocity: {
    // VELM cmds set velocity for MIDI steps
    // Ensure velocity doesn't exceed 127 (MIDI spec maximum)
    velocity_ = value & 0x7F;
  }; break;

  case FourCC::InstrumentCommandVolume: {
    MidiMessage msg;
    msg.status_ = MidiMessage::MIDI_CONTROL_CHANGE + mchannel;
    msg.data1_ = MidiCC::CC_VOLUME;
    msg.data2_ = value / 2;
    svc_->QueueMessage(msg);
  }; break;

  case FourCC::InstrumentCommandMidiCC: {
    MidiMessage msg;
    msg.status_ = MidiMessage::MIDI_CONTROL_CHANGE + mchannel;
    msg.data1_ = (value & 0x7F00) >> 8;
    msg.data2_ = (value & 0x7F);
    svc_->QueueMessage(msg);
  }; break;

  case FourCC::InstrumentCommandMidiPC: {
    SendProgramChange(mchannel, value & 0x7F);
  }; break;

  case FourCC::InstrumentCommandMidiChord: {
    // split into 4 note offsets
    for (int i = 0; i < MAX_MIDI_CHORD_NOTES; i++) {
      uint8_t noteOffset = (value >> (i * 4)) & 0xF;
      if (noteOffset == 0) {
        continue;
      }

      // fit the offset into nearest valid note of the currently selected scale
      // Add/remove from offset to match selected scale
      uint8_t rootNote = lastNotes_[channel][0];
      int scale = Player::GetInstance()->GetProject()->GetScale();
      int scaleRoot = Player::GetInstance()->GetProject()->GetScaleRoot();
      // apply current scale to offset, taking into account the scale root
      uint8_t scaledOffset = getSemitonesOffset(scale, noteOffset, scaleRoot);

      // use the existing steps note to calculate each notes offset
      uint8_t note = rootNote + scaledOffset;
      // Trace::Debug("MIDI SCALE note:%d root:%d offset: %d", note, rootNote,
      //              noteOffset);

      // save the chord note for sending a note off later
      lastNotes_[channel][i + 1] = note;

      if (noteOffset != 0) {
        MidiMessage msg;
        msg.status_ = MidiMessage::MIDI_NOTE_ON + mchannel;
        msg.data1_ = note;
        msg.data2_ = velocity_;
        // Trace::Debug("MIDI chord note ON[%d]: %d", i, msg.data1_);
        svc_->QueueMessage(msg);
      }
    }
  }; break;
  case FourCC::InstrumentCommandKill: {
    Stop(channel);
  }; break;
  }
}

etl::string<MAX_INSTRUMENT_NAME_LENGTH> MidiInstrument::GetDefaultName() {
  // use the channel number as a fallback
  Variable *v = FindVariable(FourCC::MidiInstrumentChannel);
  defaultName_ = "MIDI CH ";
  int displayChannelNum = v->GetInt() + 1;
  char channelStr[3];
  hex2char(displayChannelNum, channelStr);
  defaultName_ += channelStr;
  return defaultName_;
}

int MidiInstrument::GetTable() {
  Variable *v = FindVariable(FourCC::MidiInstrumentTable);
  return v->GetInt();
};

bool MidiInstrument::GetTableAutomation() {
  Variable *v = FindVariable(FourCC::MidiInstrumentTableAutomation);
  return v->GetBool();
};

void MidiInstrument::GetTableState(TableSaveState &state) {
  memcpy(state.hopCount_, tableState_.hopCount_,
         sizeof(uchar) * TABLE_STEPS * 3);
  memcpy(state.position_, tableState_.position_, sizeof(int) * 3);
};

void MidiInstrument::SetTableState(TableSaveState &state) {
  memcpy(tableState_.hopCount_, state.hopCount_,
         sizeof(uchar) * TABLE_STEPS * 3);
  memcpy(tableState_.position_, state.position_, sizeof(int) * 3);
};

void MidiInstrument::SendProgramChange(int channel, int program) {
  MidiMessage msg;
  msg.status_ = MidiMessage::MIDI_PROGRAM_CHANGE + channel;
  msg.data1_ = program;
  msg.data2_ = MidiMessage::UNUSED_BYTE;
  svc_->QueueMessage(msg);
};

void MidiInstrument::SendProgramChangeWithNote(int channel, int program) {
  // First send the program change
  SendProgramChange(channel, program);

  // Define C3 as MIDI note 60
  const uint8_t C3_NOTE = 60;

  // Send Note On for C3 with velocity 100 (0x64)
  MidiMessage noteOn;
  noteOn.status_ = MidiMessage::MIDI_NOTE_ON + channel;
  noteOn.data1_ = C3_NOTE;
  noteOn.data2_ = 0x64; // Velocity 100
  svc_->QueueMessage(noteOn);

  // Set up the note-off information for the callback
  NoteOffInfo::current.channel = channel;
  NoteOffInfo::current.note = C3_NOTE;

  // Schedule the note-off message after 300ms using TimerService
  // This is non-blocking and will happen asynchronously
  timerSvc_->TriggerCallback(300, NoteOffCallback);
};

void MidiInstrument::NoteOffCallback() {
  // This static callback will be called after the timer expires
  // Send the Note Off message for the stored note
  if (svc_ != nullptr) {
    MidiMessage noteOff;
    noteOff.status_ = MidiMessage::MIDI_NOTE_OFF + NoteOffInfo::current.channel;
    noteOff.data1_ = NoteOffInfo::current.note;
    noteOff.data2_ = 0x00;
    svc_->QueueMessage(noteOff);
  }
};
