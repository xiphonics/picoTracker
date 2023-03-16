
#include "PICOSerialMidiDevice.h"
#include "System/Console/Trace.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"
#include "Adapters/PICO/platform/platform.h"

PICOSerialMidiOutDevice::PICOSerialMidiOutDevice(const char *name)
    : MidiOutDevice(name) {
}

bool PICOSerialMidiOutDevice::Init() { return true; }

void PICOSerialMidiOutDevice::Close(){};

bool PICOSerialMidiOutDevice::Start() { return true; };

void PICOSerialMidiOutDevice::Stop() {}

void PICOSerialMidiOutDevice::SendMessage(MidiMessage &msg) {
  uart_putc_raw(MIDI_UART, msg.status_);

  if (msg.status_ < 0xF0) {
    uart_putc_raw(MIDI_UART, msg.data1_);
    if (msg.data2_ != MidiMessage::UNUSED_BYTE) {
      uart_putc_raw(MIDI_UART, msg.data2_);
    }
  }
}
