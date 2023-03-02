#ifndef _PICO_SERIAL_MIDI_DEVICE_H_
#define _PICO_SERIAL_MIDI_DEVICE_H_

#include "Services/Midi/MidiOutDevice.h"
#include "hardware/uart.h"

class PICOSerialMidiOutDevice : public MidiOutDevice {
public:
  PICOSerialMidiOutDevice(const char *name, uart_inst_t *port_);
  virtual bool Init();
  virtual void Close();
  virtual bool Start();
  virtual void Stop();

protected:
  virtual void SendMessage(MidiMessage &);

private:
  uart_inst_t *port_;
};
#endif
