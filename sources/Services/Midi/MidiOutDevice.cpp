#include "MidiOutDevice.h"

MidiOutDevice::MidiOutDevice(const char *name) { name_ = name; };

MidiOutDevice::~MidiOutDevice(){};

const char *MidiOutDevice::GetName() { return name_.c_str(); };

void MidiOutDevice::SetName(const char *name) { name_ = name; }

void MidiOutDevice::SendQueue(etl::vector<MidiMessage, 10> &queue) {
  for (auto &msg : queue) {
    SendMessage(msg);
  }
}
