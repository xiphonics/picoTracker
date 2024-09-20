
#include "picoTrackerMidiService.h"
#include "picoTrackerMidiDevice.h"

picoTrackerMidiService::picoTrackerMidiService(){};

picoTrackerMidiService::~picoTrackerMidiService(){};

void picoTrackerMidiService::buildDriverList() {
  // create a midi device for each of Midi Output device
  // for now only 1 TRS midi output
  outList_.Insert(new picoTrackerMidiOutDevice("MIDI OUT 1"));
};
