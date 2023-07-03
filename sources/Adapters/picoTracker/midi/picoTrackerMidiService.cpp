
#include "picoTrackerMidiService.h"
#include "picoTrackerMidiInDevice.h"
#include "picoTrackerMidiOutDevice.h"

picoTrackerMidiService::picoTrackerMidiService(){};

picoTrackerMidiService::~picoTrackerMidiService(){};

void picoTrackerMidiService::buildDriverList() { // Here we just loop over
                                                 // existing
  // Midi out and create a midi device
  // for each of them.

  inList_.Insert(new picoTrackerMidiInDevice("MIDI IN 1"));
  Insert(new picoTrackerMidiOutDevice("MIDI OUT 1"));
};
