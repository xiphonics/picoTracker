
#include "picoTrackerMidiInDevice.h"
#include "Adapters/picoTracker/platform/platform.h"
#include "System/Console/Trace.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"

picoTrackerMidiInDevice *picoTrackerMidiInDevice::instance_ = NULL;

void rxCallback() {
  MidiMessage msg;

  // Assume if this was called, then there is at least one message in FIFO queue
  msg.status_ = uart_getc(MIDI_UART);
  msg.data1_ = uart_is_readable(MIDI_UART) ? uart_getc(MIDI_UART) : 0;
  msg.data2_ = uart_is_readable(MIDI_UART) ? uart_getc(MIDI_UART) : 0;
  picoTrackerMidiInDevice::sendDriverMessage(msg);
}

void picoTrackerMidiInDevice::sendDriverMessage(MidiMessage &message) {
  instance_->onDriverMessage(message);
}

picoTrackerMidiInDevice::picoTrackerMidiInDevice(const char *name)
    : MidiInDevice(name) {}

picoTrackerMidiInDevice::~picoTrackerMidiInDevice(){};

bool picoTrackerMidiInDevice::initDriver() {

  instance_ = this;

  // most port initialization happens in  platform initialization phase
  irq_set_exclusive_handler(MIDI_UART_IRQ, rxCallback);
  irq_set_enabled(MIDI_UART_IRQ, true);
  uart_set_irq_enables(MIDI_UART, true, false);

  return true;
}

void picoTrackerMidiInDevice::closeDriver(){};

bool picoTrackerMidiInDevice::startDriver() { return true; };

void picoTrackerMidiInDevice::stopDriver() {}
