#include "picoTrackerMidiInDevice.h"
#include "Adapters/picoTracker/platform/platform.h"
#include "System/Console/Trace.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"

#define MASK3 0b11100000
#define MASK5 0b11111000

#define SINGLEDATABYTE 0b11000000
#define REALTIME 0b11111000
#define IGNORE 0b11110000

picoTrackerMidiInDevice *picoTrackerMidiInDevice::instance_ = NULL;
static unsigned char status = 0;
static unsigned char dataBytes = 0;
static unsigned char prevData = 255;

void rxCallback() {

  // Assume if this was called, then there is at least one message in FIFO
  // queue no need to use uart_is_readable(MIDI_UART)?

  // ref:
  // https://www.midi.org/specifications/midi-reference-tables/summary-of-midi-1-0-messages

  // TL;DR
  // 0b1xxxxxxx - Status byte (2 data bytes with exceptions)
  // 0b110xxxxx - Program change and after touch (1 data byte)
  // 0b11111xxx - Real time messages (0 data bytes, no running status)
  // 0b11110xxx - System common messages (variable data bytes) - we'll ignore
  // these 0b0xxxxxxx - Data byte Check bit 7 for status vs data check bit 5
  // for 2 data byte vs 1 data byte check bit 3 for RT vs System common

  printf("enter callback\n");
  unsigned char byte = uart_getc(MIDI_UART);
  // unsigned char byte = uart_is_readable(MIDI_UART) ? uart_getc(MIDI_UART) : 0;
  printf("----------------------\n");
  printf("%02X\n", byte);
  if (byte >> 7) {
    printf("It's status byte\n");
    // it's status byte
    if ((byte & MASK3) == SINGLEDATABYTE) {
      printf("It's single data\n");
      // 1 Data byte
      status = byte;
      dataBytes = 1;
    } else if ((byte & MASK5) == REALTIME) {
      printf("It's real time\n");
      // Real time message (0b11111xxx), send right away
      status = 0; // running status not expected for RT msgs
      dataBytes = 0; // we don't expect any data bytes after this
      MidiMessage msg = MidiMessage(byte, 0, 0);
      picoTrackerMidiInDevice::sendDriverMessage(msg);
    } else if ((byte & MASK5) == IGNORE) {
      // Ignore System common messages (0b11110xxx)
      printf("It's ignore\n");
      status = 0;
      dataBytes = 0;
    } else {
      printf("It's two data\n");
      // Everything else, 2 data bytes
      status = byte;
      dataBytes = 2;
    }
  } else {
    printf("It's data byte\n");
    // It's data byte
    switch (dataBytes) {
    case 2:
      {
        printf("First byte of two data\n");
        if (prevData == 255) {
          // we don't have prev data, this is the first one
          prevData = byte;
        } else {
          printf("Second byte of two data, send midi msg: %02X %02X %02X\n", status, prevData, byte);
          // This is the second byte
          MidiMessage msg = MidiMessage(status, prevData, byte);
          picoTrackerMidiInDevice::sendDriverMessage(msg);
          // reset prev data
          prevData = 255;
        }
        break;
      }
    case 1:
      {
        printf("First byte of one data, send midi msg: %02X %02X %02X\n", status, byte, 0);
        MidiMessage msg = MidiMessage(status, byte, 0);
        picoTrackerMidiInDevice::sendDriverMessage(msg);
        break;
      }
    default:
      printf("Ignore data since it doesn't correspond to a valid status byte\n");
      // Do nothing, a data byte after a status that doesn't expect it is ignored
      break;
    }
  }
  printf("----------------------");
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
