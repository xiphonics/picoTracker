#ifndef _PICOTRACKERMIDISERVICE_H_
#define _PICOTRACKERMIDISERVICE_H_

#include "Services/Midi/MidiService.h"

class picoTrackerMidiService : public MidiService {
public:
  picoTrackerMidiService();
  ~picoTrackerMidiService();

  // Poll MIDI input devices for new messages
  void poll();
};

#endif
