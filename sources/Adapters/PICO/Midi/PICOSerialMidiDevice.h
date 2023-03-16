#ifndef _PICO_SERIAL_MIDI_DEVICE_H_
#define _PICO_SERIAL_MIDI_DEVICE_H_

#include "Services/Midi/MidiOutDevice.h"

class PICOSerialMidiOutDevice : public MidiOutDevice {
public:
  PICOSerialMidiOutDevice(const char *name);
  virtual bool Init();
  virtual void Close();
  virtual bool Start();
  virtual void Stop();

protected:
  virtual void SendMessage(MidiMessage &);

private:
};
#endif
