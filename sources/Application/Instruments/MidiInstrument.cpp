#include "MidiInstrument.h"
#include "CommandList.h"
#include "Externals/etl/include/etl/to_string.h"
#include "System/Console/Trace.h"
#include <string.h>

#define MAX_MIDI_CHORD_NOTES 4

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
  lastNote_[c] = note;

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

  MidiMessage msg;
  msg.status_ = MIDI_NOTE_OFF + channel;
  msg.data1_ = lastNote_[c];
  msg.data2_ = 0x00;
  svc_->QueueMessage(msg);
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
    msg.data1_ = lastNote_[channel];
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
        msg.data1_ = lastNote_[channel];
        msg.data2_ = 0x00;
        svc_->QueueMessage(msg);
        msg.status_ = MIDI_NOTE_ON + mchannel;
        msg.data1_ = lastNote_[channel];
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
      uint8_t note = (value >> (i * 4)) & 0xF;
      if (note != 0) {
        MidiMessage msg;
        msg.status_ = MIDI_NOTE_ON + mchannel;
        msg.data1_ = lastNote_[channel] + note;
        msg.data2_ = velocity_;
        Trace::Log("MIDI", "chord note %d", msg.data1_);
        svc_->QueueMessage(msg);
      }
    }
  }; break;
  };
}

etl::string<24> MidiInstrument::GetName() {
  Variable *v = FindVariable(FourCC::MidiInstrumentChannel);
  etl::string<24> name = "MIDI CH ";
  etl::to_string(v->GetInt() + 1, name, etl::format_spec().precision(0), true);
  return name;
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
