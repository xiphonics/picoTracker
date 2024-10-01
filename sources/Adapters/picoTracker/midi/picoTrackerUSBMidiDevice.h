#ifndef _PICOTRACKERUSBMIDIDEVICE_H_
#define _PICOTRACKERUSBMIDIDEVICE_H_

#include "Services/Midi/MidiOutDevice.h"

class picoTrackerUSBMidiOutDevice : public MidiOutDevice {
public:
  picoTrackerUSBMidiOutDevice(const char *name);
  virtual bool Init();
  virtual void Close();
  virtual bool Start();
  virtual void Stop();

protected:
  virtual void SendMessage(MidiMessage &);

private:
};
#endif