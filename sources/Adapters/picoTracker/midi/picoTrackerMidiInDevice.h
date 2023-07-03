#ifndef _PICOTRACKERMIDIINDEVICE_H_
#define _PICOTRACKERMIDIINDEVICE_H_

#include "Services/Midi/MidiInDevice.h"

class picoTrackerMidiInDevice : public MidiInDevice {
public:
  picoTrackerMidiInDevice(const char *name);
  ~picoTrackerMidiInDevice();

  static void sendDriverMessage(MidiMessage &message);

protected:
  // Driver specific initialisation

  virtual bool initDriver();
  virtual void closeDriver();
  virtual bool startDriver();
  virtual void stopDriver();

private:
  static picoTrackerMidiInDevice *instance_;
};
#endif
