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
      table_(FourCC::MidiInstrumentTable, -1),
      tableAuto_(FourCC::MidiInstrumentTableAutomation, false),
      program_(FourCC::MidiInstrumentProgram, 0) {

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

void MidiInstrument::OnStart() { tableState_.Reset(); };

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

  MidiMessage msg;

  // send instrument volume for this midi channel when it's not zero
  v = FindVariable(FourCC::MidiInstrumentVolume);
  int volume = v->GetInt();
  if (volume > 0) {
    msg.status_ = MidiMessage::MIDI_CONTROL_CHANGE + channel;
    msg.data1_ = MidiCC::CC_VOLUME;
    msg.data2_ = volume / 2;
    svc_->QueueMessage(msg);
  }

  // send program change message
  v = FindVariable(FourCC::MidiInstrumentProgram);
  int program = v->GetInt();
  SendProgramChange(channel, program);

  // set initial velocity (changed via InstrumentCommandVelocity)
  velocity_ = INITIAL_NOTE_VELOCITY;
  playing_ = true;
  retrig_ = false;

  return true;
};

void MidiInstrument::Stop(int c) {

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
}

void MidiInstrument::HandleMuteChange(int c, bool muted) {
  if (muted) {
    // When a channel becomes muted, send note-off messages for all active notes
    Variable *v = FindVariable(FourCC::MidiInstrumentChannel);
    int channel = v->GetInt();

    // First send individual note-off messages for any active notes
    for (int i = 0; i < MAX_MIDI_CHORD_NOTES + 1; i++) {
      if (lastNotes_[c][i] == 0) {
        continue;
      }
      MidiMessage msg;
      msg.status_ = MidiMessage::MIDI_NOTE_OFF + channel;
      msg.data1_ = lastNotes_[c][i];
      msg.data2_ = 0x00;
      svc_->QueueMessage(msg);
      Trace::Debug("MIDI mute note OFF[%d]:%d", i, msg.data1_);
    }
  }
  // No action needed when unmuting - notes will play on next trigger
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

  // Check if the channel is muted before sending MIDI messages
  Player *player = Player::GetInstance();
  bool channelMuted = player->IsChannelMuted(channel);

  if (first_[channel] && !channelMuted) {
    // Only send note-on if the channel is not muted
    MidiMessage msg;
    msg.status_ = MidiMessage::MIDI_NOTE_ON + mchannel;
    msg.data1_ = lastNotes_[channel][0];
    msg.data2_ = velocity_;
    svc_->QueueMessage(msg);

    first_[channel] = false;
  } else if (first_[channel] && channelMuted) {
    // If channel is muted, don't send note-on but mark as not first anymore
    first_[channel] = false;
  }
  if (remainingTicks_ > 0) {
    remainingTicks_--;
    if (remainingTicks_ == 0) {
      if (!retrig_) {
        Stop(channel);
      } else if (!channelMuted) {
        // Only send retrigger MIDI messages if the channel is not muted
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
      } else {
        // If muted, just reset the timer without sending MIDI messages
        remainingTicks_ = retrigLoop_;
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

  case FourCC::InstrumentCommandVelocity: {
    // VELM cmds set velocity for MIDI steps
    velocity_ = value / 2;
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
      // apply current scale to offset
      uint8_t scaledOffset = getSemitonesOffset(scale, noteOffset);

      // use the existing steps note to calculate each notes offset
      uint8_t note = rootNote + scaledOffset;
      Trace::Debug("MIDI SCALE note:%d root:%d offset: %d", note, rootNote,
                   noteOffset);

      // save the chord note for sending a note off later
      lastNotes_[channel][i + 1] = note;

      if (noteOffset != 0) {
        MidiMessage msg;
        msg.status_ = MidiMessage::MIDI_NOTE_ON + mchannel;
        msg.data1_ = note;
        msg.data2_ = velocity_;
        Trace::Debug("MIDI chord note ON[%d]: %d", i, msg.data1_);
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