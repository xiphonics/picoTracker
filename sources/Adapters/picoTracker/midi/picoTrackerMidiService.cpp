
#include "picoTrackerMidiService.h"
#include "picoTrackerMidiDevice.h"

picoTrackerMidiService::picoTrackerMidiService(){};

picoTrackerMidiService::~picoTrackerMidiService(){};

void picoTrackerMidiService::buildDriverList() {
  // create the one and only MIDI out device on the pico
  // the MidiService parent class will fish out this as the first item
  // of the list
  Insert(new picoTrackerMidiOutDevice("MIDI OUT 1"));
};