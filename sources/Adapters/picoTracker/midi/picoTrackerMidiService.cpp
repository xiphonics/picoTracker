
#include "picoTrackerMidiService.h"
#include "picoTrackerMidiDevice.h"

picoTrackerMidiService::picoTrackerMidiService(){};

picoTrackerMidiService::~picoTrackerMidiService(){};

void picoTrackerMidiService::buildDriverList() {
  // create just the TRS MIDI out device on the pico for now and
  // the MidiService parent class will fish out this out of the list that
  // implemented by this class
  Insert(new picoTrackerMidiOutDevice("MIDI OUT 1"));
};