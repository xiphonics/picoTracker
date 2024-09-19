#include "MidiInstrument.h"
#include "CommandList.h"
#include "Externals/etl/include/etl/to_string.h"
#include "System/Console/Trace.h"
#include <string.h>

MidiService *MidiInstrument::svc_ = 0;

MidiInstrument::MidiInstrument() {

  // Reserve Observer
  ReserveObserver(1);

  if (svc_ == 0) {
    svc_ = MidiService::GetInstance();
  };

  Variable *v = new Variable("channel", MIP_CHANNEL, 0);
  insert(end(), v);
  v = new Variable("note length", MIP_NOTELENGTH, 0);
  insert(end(), v);
  v = new Variable("volume", MIP_VOLUME, 255);
  insert(end(), v);
  v = new Variable("table", MIP_TABLE, -1);
  insert(end(), v);
  v = new Variable("table automation", MIP_TABLEAUTO, false);
  insert(end(), v);
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

  Variable *v = FindVariable(MIP_CHANNEL);
  int channel = v->GetInt();

  v = FindVariable(MIP_NOTELENGTH);
  remainingTicks_ = v->GetInt();
  if (remainingTicks_ == 0) {
    remainingTicks_ = -1;
  }

  MidiMessage msg;

  //	send volume initial volume for this midi channel
  v = FindVariable(MIP_VOLUME);
  msg.status_ = MIDI_CC + channel;
  msg.data1_ = 7;
  msg.data2_ = floor(v->GetInt() / 2);
  svc_->QueueMessage(msg);

  // store initial velocity
  velocity_ = msg.data2_;
  playing_ = true;
  retrig_ = false;
  porto_ = false;

  return true;
};

void MidiInstrument::Stop(int c) {
  char note = lastNote_[c];
  if (porto_) {
    porto_ = false;
    porto_pending_[c] = note;
    return;
  }

  Variable *v = FindVariable(MIP_CHANNEL);
  int channel = v->GetInt();

  MidiMessage msg;
  msg.status_ = MIDI_NOTE_OFF + channel;
  msg.data1_ = note;
  msg.data2_ = 0x00;
  svc_->QueueMessage(msg);
  playing_ = false;
};

void MidiInstrument::SetChannel(int channel) {
  Variable *v = FindVariable(MIP_CHANNEL);
  v->SetInt(channel);
};

bool MidiInstrument::Render(int channel, fixed *buffer, int size,
                            bool updateTick) {

  // We do it here so we have the opportunity to send some command before

  Variable *v = FindVariable(MIP_CHANNEL);
  int mchannel = v->GetInt();
  if (first_[channel]) {
    // send note
    MidiMessage msg;

    msg.status_ = MIDI_NOTE_ON + mchannel;
    msg.data1_ = lastNote_[channel];
    msg.data2_ = velocity_;
    svc_->QueueMessage(msg);

    first_[channel] = false;
    if (porto_pending_[channel] > 0 &&
        porto_pending_[channel] != lastNote_[channel]) {
      // now we've started playing the current note, need to stop the pending
      // protomento prev note, can't just use Stop() as it uses last_note
      // directly
      int channel = v->GetInt();
      MidiMessage msg;
      msg.status_ = MIDI_NOTE_OFF + channel;
      msg.data1_ = porto_pending_[channel];
      msg.data2_ = 0x00;
      svc_->QueueMessage(msg);
      porto_pending_[channel] = 0;
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

  Variable *v = FindVariable(MIP_CHANNEL);
  int mchannel = v->GetInt();

  switch (cc) {

  case I_CMD_RTRG: {
    unsigned char loop = (value & 0xFF); // number of ticks before repeat
    if (loop != 0) {
      retrig_ = true;
      retrigLoop_ = loop;
      remainingTicks_ = loop;
    } else {
      retrig_ = false;
    }
  } break;

  case I_CMD_VELM: {
    // VELM cmds set velocity for MIDI steps
    velocity_ = floor(value / 2);
  }; break;

  case I_CMD_VOLM: {
    MidiMessage msg;
    msg.status_ = MIDI_CC + mchannel;
    msg.data1_ = 7;
    msg.data2_ = floor(value / 2);
    svc_->QueueMessage(msg);
  }; break;

  case I_CMD_MDCC: {
    MidiMessage msg;
    msg.status_ = MIDI_CC + mchannel;
    msg.data1_ = (value & 0x7F00) >> 8;
    msg.data2_ = (value & 0x7F);
    svc_->QueueMessage(msg);
  }; break;

  case I_CMD_MDPG: {
    MidiMessage msg;
    msg.status_ = MIDI_PRG + mchannel;
    msg.data1_ = (value & 0x7F);
    msg.data2_ = MidiMessage::UNUSED_BYTE;
    svc_->QueueMessage(msg);
  }; break;
  case I_CMD_PTCH: {
    // For MIDI this is portomento, ie, dont stop prev note before starting
    // next one
    porto_ = true;
  } break;
  }
};

etl::string<24> MidiInstrument::GetName() {
  Variable *v = FindVariable(MIP_CHANNEL);
  etl::string<24> name = "MIDI CH ";
  etl::to_string(v->GetInt() + 1, name, etl::format_spec().precision(0), true);
  return name;
}

int MidiInstrument::GetTable() {
  Variable *v = FindVariable(MIP_TABLE);
  return v->GetInt();
};

bool MidiInstrument::GetTableAutomation() {
  Variable *v = FindVariable(MIP_TABLEAUTO);
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
