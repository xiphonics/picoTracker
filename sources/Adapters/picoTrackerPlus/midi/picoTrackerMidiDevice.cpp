
#include "picoTrackerMidiDevice.h"
#include "Adapters/picoTrackerPlus/platform/platform.h"
#include "System/Console/Trace.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"

picoTrackerMidiOutDevice::picoTrackerMidiOutDevice(const char *name)
    : MidiOutDevice(name) {}

bool picoTrackerMidiOutDevice::Init() { return true; }

void picoTrackerMidiOutDevice::Close(){};

bool picoTrackerMidiOutDevice::Start() { return true; };

void picoTrackerMidiOutDevice::Stop() {}

void picoTrackerMidiOutDevice::SendMessage(MidiMessage &msg) {
  uart_putc_raw(MIDI_UART, msg.status_);

  if (msg.status_ < 0xF0) {
    uart_putc_raw(MIDI_UART, msg.data1_);
    if (msg.data2_ != MidiMessage::UNUSED_BYTE) {
      uart_putc_raw(MIDI_UART, msg.data2_);
    }
  }
}
