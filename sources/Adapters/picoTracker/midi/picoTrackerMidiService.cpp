
#include "picoTrackerMidiService.h"
#include "picoTrackerMidiDevice.h"
#include "picoTrackerUSBMidiDevice.h"

picoTrackerMidiService::picoTrackerMidiService(){};

picoTrackerMidiService::~picoTrackerMidiService(){};

void picoTrackerMidiService::buildDriverList() {
  // create a midi device for each of Midi Output device
  MidiOutDevice *dev = new picoTrackerMidiOutDevice("MIDI OUT 1");
  outList_.insert(outList_.end(), dev);
  dev = new picoTrackerUSBMidiOutDevice("USB");
  outList_.insert(outList_.end(), dev);
};
