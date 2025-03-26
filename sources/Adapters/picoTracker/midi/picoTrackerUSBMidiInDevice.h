#ifndef _PICOTRACKERUSBMIDIINDEVICE_H_
#define _PICOTRACKERUSBMIDIINDEVICE_H_

#include "Services/Midi/MidiInDevice.h"
#include "tusb.h"

class picoTrackerUSBMidiInDevice : public MidiInDevice {
public:
  picoTrackerUSBMidiInDevice(const char *name);
  virtual ~picoTrackerUSBMidiInDevice();

  virtual bool Init();
  virtual void Close();
  virtual bool Start();
  virtual void Stop();

  // Process any pending USB MIDI messages
  virtual void poll();

protected:
  virtual bool initDriver() { return true; };
  virtual bool startDriver() { return true; };
  virtual void stopDriver() {}
  virtual void closeDriver() {}
};

#endif