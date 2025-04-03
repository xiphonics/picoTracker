#ifndef _PICOTRACKERMIDISERVICE_H_
#define _PICOTRACKERMIDISERVICE_H_

#include "Services/Midi/MidiService.h"
#include "picoTrackerMidiInDevice.h"
#include "picoTrackerMidiOutDevice.h"
#include "picoTrackerUSBMidiInDevice.h"
#include "picoTrackerUSBMidiOutDevice.h"

class picoTrackerMidiService : public MidiService {
public:
  picoTrackerMidiService();
  ~picoTrackerMidiService();

  // Poll MIDI input devices for new messages
  void poll();

private:
  picoTrackerMidiOutDevice midiOutDevice_;
  picoTrackerUSBMidiOutDevice usbMidiOutDevice_;
  picoTrackerMidiInDevice midiInDevice_;
  picoTrackerUSBMidiInDevice usbMidiInDevice_;
};

#endif
