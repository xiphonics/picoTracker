#include "MidiOutDevice.h"

MidiOutDevice::MidiOutDevice(const char *name) { name_ = name; };

MidiOutDevice::~MidiOutDevice(){};

const char *MidiOutDevice::GetName() { return name_.c_str(); };

void MidiOutDevice::SetName(const char *name) { name_ = name; }

void MidiOutDevice::SendQueue(T_SimpleList<MidiMessage> &queue) {

  for (queue.Begin(); !queue.IsDone(); queue.Next()) {
    MidiMessage msg = queue.CurrentItem();
    SendMessage(msg);
  };
}
