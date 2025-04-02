#include "MidiInstrument.h"
#include "Application/Model/Scale.h"
#include "Application/Player/Player.h"
#include "Application/Utils/char.h"
#include "CommandList.h"
#include "System/Console/Trace.h"

MidiService *MidiInstrument::svc_ = 0;

MidiInstrument::MidiInstrument()
    : I_Instrument(&variables_), channel_(FourCC::MidiInstrumentChannel, 0),
      noteLen_(FourCC::MidiInstrumentNoteLength, 0),
      volume_(FourCC::MidiInstrumentVolume, 255),
      table_(FourCC::MidiInstrumentTable, -1),
      tableAuto_(FourCC::MidiInstrumentTableAutomation, false) {

  if (svc_ == 0) {
    svc_ = MidiService::GetInstance();
  };

  variables_.insert(variables_.end(), &name_);
  variables_.insert(variables_.end(), &channel_);
  variables_.insert(variables_.end(), &noteLen_);
  variables_.insert(variables_.end(), &volume_);
  variables_.insert(variables_.end(), &table_);
  variables_.insert(variables_.end(), &tableAuto_);
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

  //	send volume initial volume for this midi channel
  v = FindVariable(FourCC::MidiInstrumentVolume);
  msg.status_ = MIDI_CC + channel;
  msg.data1_ = 7;
  msg.data2_ = floor(v->GetInt() / 2);
  svc_->QueueMessage(msg);

  // store initial velocity
  velocity_ = msg.data2_;
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
    msg.status_ = MIDI_NOTE_OFF + channel;
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
    msg.status_ = MIDI_NOTE_ON + mchannel;
    msg.data1_ = lastNotes_[channel][0];
    msg.data2_ = velocity_;
    svc_->QueueMessage(msg);

    first_[channel] = false;
  }
  if (remainingTicks_ > 0) {
    remainingTicks_--;
    if (remainingTicks_ == 0) {
      if (!retrig_) {
        Stop(channel);
      } else {
        MidiMessage msg;
        remainingTicks_ = retrigLoop_;
        msg.status_ = MIDI_NOTE_OFF + mchannel;
        msg.data1_ = lastNotes_[channel][0];
        msg.data2_ = 0x00;
        svc_->QueueMessage(msg);
        msg.status_ = MIDI_NOTE_ON + mchannel;
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

  case FourCC::InstrumentCommandVelocity: {
    // VELM cmds set velocity for MIDI steps
    velocity_ = floor(value / 2);
  }; break;

  case FourCC::InstrumentCommandVolume: {
    MidiMessage msg;
    msg.status_ = MIDI_CC + mchannel;
    msg.data1_ = 7;
    msg.data2_ = floor(value / 2);
    svc_->QueueMessage(msg);
  }; break;

  case FourCC::InstrumentCommandMidiCC: {
    MidiMessage msg;
    msg.status_ = MIDI_CC + mchannel;
    msg.data1_ = (value & 0x7F00) >> 8;
    msg.data2_ = (value & 0x7F);
    svc_->QueueMessage(msg);
  }; break;

  case FourCC::InstrumentCommandMidiPC: {
    MidiMessage msg;
    msg.status_ = MIDI_PRG + mchannel;
    msg.data1_ = (value & 0x7F);
    msg.data2_ = MidiMessage::UNUSED_BYTE;
    svc_->QueueMessage(msg);
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
        msg.status_ = MIDI_NOTE_ON + mchannel;
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

etl::string<MAX_INSTRUMENT_NAME_LENGTH> MidiInstrument::GetName() {
  // use the channel number as a fallback
  Variable *v = FindVariable(FourCC::MidiInstrumentChannel);
  etl::string<MAX_INSTRUMENT_NAME_LENGTH> temp = "MIDI CH ";

  int displayChannelNum = v->GetInt() + 1;
  char channelStr[3];
  hex2char(displayChannelNum, channelStr);
  temp += channelStr;

  name_.SetString(temp.c_str());
  return name_.GetString();
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