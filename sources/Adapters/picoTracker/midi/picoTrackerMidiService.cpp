
#include "picoTrackerMidiService.h"
#include "picoTrackerMidiDevice.h"

picoTrackerMidiService::picoTrackerMidiService(){};

picoTrackerMidiService::~picoTrackerMidiService(){};

void picoTrackerMidiService::buildDriverList() { // Here we just loop over
                                                 // existing
  // Midi out and create a midi device
  // for each of them.

  Insert(new picoTrackerMidiOutDevice("MIDI OUT 1"));
};
