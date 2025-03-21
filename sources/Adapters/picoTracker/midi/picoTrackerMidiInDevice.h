// picoTrackerMidiInDevice.h
#pragma once
#include "Services/Midi/MidiInDevice.h"

class picoTrackerMidiInDevice : public MidiInDevice {
public:
  picoTrackerMidiInDevice(const char *name);
  virtual ~picoTrackerMidiInDevice();

  // Poll the MIDI input buffer for new messages
  void poll();

  virtual bool Start();
  virtual void Stop();

protected:
  // Driver specific implementation
  virtual bool initDriver();
  virtual void closeDriver();
  virtual bool startDriver();
  virtual void stopDriver();

  // MIDI message parsing
  void processMidiData(uint8_t data);
};