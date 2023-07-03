#ifndef _PICOTRACKERMIDIOUTDEVICE_H_
#define _PICOTRACKERMIDIOUTDEVICE_H_

#include "Services/Midi/MidiOutDevice.h"

class picoTrackerMidiOutDevice : public MidiOutDevice {
public:
  picoTrackerMidiOutDevice(const char *name);
  virtual bool Init();
  virtual void Close();
  virtual bool Start();
  virtual void Stop();

protected:
  virtual void SendMessage(MidiMessage &);

private:
};
#endif
