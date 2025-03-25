#include "picoTrackerMidiInDevice.h"
#include "Adapters/picoTracker/platform/platform.h"
#include "Services/Midi/MidiMessage.h"
#include "System/Console/Trace.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/sync.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"

// Ring buffer size for MIDI input
#define MIDI_UART_BUFFER_SIZE 128

// Static pointer to the MIDI device instance
static picoTrackerMidiInDevice *g_midiInDevice = nullptr;

// MIDI message parsing state
static uint8_t midiStatus = 0;
static uint8_t midiData1 = 0;
static uint8_t midiDataCount = 0;
static uint8_t midiDataBytes = 0;

// Ring buffer for MIDI input
static uint8_t midi_rx_buffer[MIDI_UART_BUFFER_SIZE];
static volatile uint32_t midi_rx_head = 0;
static volatile uint32_t midi_rx_tail = 0;

// UART interrupt handler - called directly from the IRQ
void __isr __time_critical_func(midi_uart_irq_handler)() {
  uint32_t status = uart_get_hw(MIDI_UART)->mis; // the reason this irq happened

  // Check if this is a UART RX interrupt
  if (status & UART_UARTMIS_RXMIS_BITS || status & UART_UARTMIS_RTMIS_BITS) {
    // Process all available data
    while (uart_is_readable(MIDI_UART)) {
      uint8_t data = uart_getc(MIDI_UART);
      // Store in ring buffer
      uint32_t next_head = (midi_rx_head + 1) % MIDI_UART_BUFFER_SIZE;
      if (next_head != midi_rx_tail) {
        midi_rx_buffer[midi_rx_head] = data;
        midi_rx_head = next_head;
      }
    }
  }
}

picoTrackerMidiInDevice::picoTrackerMidiInDevice(const char *name)
    : MidiInDevice(name) {
  g_midiInDevice = this;
  Trace::Log("MIDI", "Created MIDI input device %s", name);
}

picoTrackerMidiInDevice::~picoTrackerMidiInDevice() {
  g_midiInDevice = nullptr;
  closeDriver();
}

bool picoTrackerMidiInDevice::initDriver() {
  // UART should already be initialized in platform.cpp
  Trace::Log("MIDI", "Initialized MIDI input driver");
  return true;
}

void picoTrackerMidiInDevice::closeDriver() {
  // Disable UART RX interrupt
  irq_set_enabled(MIDI_UART_IRQ, false);
  uart_set_irq_enables(MIDI_UART, false, false);
  Trace::Log("MIDI", "Closed MIDI input driver");
}

bool picoTrackerMidiInDevice::Start() { return startDriver(); }

void picoTrackerMidiInDevice::Stop() { stopDriver(); }

bool picoTrackerMidiInDevice::startDriver() {

  // Reset the ring buffer and MIDI parser state
  midi_rx_head = 0;
  midi_rx_tail = 0;
  midiStatus = 0;
  midiDataCount = 0;

  // Set up the interrupt handler
  irq_set_exclusive_handler(MIDI_UART_IRQ, midi_uart_irq_handler);
  irq_set_enabled(MIDI_UART_IRQ, true);

  // Enable UART RX interrupt
  uart_set_irq_enables(MIDI_UART, true, false);

  Trace::Log("MIDI", "Started MIDI input driver on UART%d (IRQ: %d, Baud: %d)",
             MIDI_UART == uart0 ? 0 : 1, MIDI_UART_IRQ, MIDI_BAUD_RATE);
  return true;
}

void picoTrackerMidiInDevice::stopDriver() {
  // Disable UART RX interrupt
  irq_set_enabled(MIDI_UART_IRQ, false);
  uart_set_irq_enables(MIDI_UART, false, false);
  Trace::Log("MIDI", "Stopped MIDI input driver");
}

void picoTrackerMidiInDevice::poll() {
  uint8_t data[MIDI_UART_BUFFER_SIZE];
  bool has_data;

  // Check if there's any data to process
  // Critical section - disable interrupts while checking/updating shared
  // variables
  uint32_t save = save_and_disable_interrupts();
  has_data = (midi_rx_head != midi_rx_tail);

  if (has_data) {
    Trace::Debug("Processing MIDI data: head=%d, tail=%d", midi_rx_head,
                 midi_rx_tail);
  }
  // Process any data in the ring buffer
  int i = 0;
  while (true) {
    if (midi_rx_head == midi_rx_tail) {
      break;
    }

    // Get data and update tail
    data[i] = midi_rx_buffer[midi_rx_tail];
    midi_rx_tail = (midi_rx_tail + 1) % MIDI_UART_BUFFER_SIZE;
    i++;
  }
  restore_interrupts(save);

  for (int j = 0; j < i; j++) {
    // Process the MIDI data (outside critical section)
    Trace::Debug("Processing byte: 0x%02X", data[j]);
    processMidiData(data[j]);
  }
}

void picoTrackerMidiInDevice::processMidiData(uint8_t data) {
  // Handle MIDI data byte
  if (data & 0x80) {
    // This is a status byte

    // System Real-Time messages can appear anywhere and have no data bytes
    if (data >= 0xF8) {
      MidiMessage msg;
      msg.status_ = data;
      msg.data1_ = MidiMessage::UNUSED_BYTE;
      msg.data2_ = MidiMessage::UNUSED_BYTE;
      MidiInDevice::onDriverMessage(msg);
      return; // Don't change the running status
    }

    // For all other status bytes, reset the data count
    midiStatus = data;
    midiDataCount = 0;

    // Determine how many data bytes to expect based solely on the status byte
    if (data >= 0xF0) {
      // System Common messages
      switch (data) {
      case 0xF1: // MIDI Time Code Quarter Frame
      case 0xF3: // Song Select
        midiDataBytes = 1;
        break;
      case 0xF2: // Song Position Pointer
        midiDataBytes = 2;
        break;
      case 0xF0: // Start of System Exclusive
        // SysEx messages are variable length and end with 0xF7
        // For simplicity, we're not fully handling SysEx here
        midiDataBytes = 0; // Special case, handled differently
        break;
      default:
        // All other System Common messages have no data bytes
        midiDataBytes = 0;

        // For messages with no data bytes, send them immediately
        MidiMessage msg;
        msg.status_ = midiStatus;
        msg.data1_ = MidiMessage::UNUSED_BYTE;
        msg.data2_ = MidiMessage::UNUSED_BYTE;
        MidiInDevice::onDriverMessage(msg);
        break;
      }
    } else {
      // Channel messages - determine bytes by status byte range
      uint8_t msgType = data & 0xF0;

      // Program Change and Channel Pressure have 1 data byte
      if (msgType == 0xC0 || msgType == 0xD0) {
        midiDataBytes = 1;
      } else {
        // All other channel messages have 2 data bytes
        midiDataBytes = 2;
      }
    }
  } else {
    // This is a data byte
    if (midiStatus == 0) {
      // Ignore data bytes without status
      Trace::Debug("MIDI", "Ignored data byte without status: 0x%02X", data);
      return;
    }

    if (midiDataCount == 0) {
      midiData1 = data;
      midiDataCount++;

      // If we only expect one data byte, we have a complete message
      if (midiDataBytes == 1) {
        MidiMessage msg;
        msg.status_ = midiStatus;
        msg.data1_ = midiData1;
        msg.data2_ = MidiMessage::UNUSED_BYTE;
        MidiInDevice::onDriverMessage(msg);
      }
    } else if (midiDataCount == 1 && midiDataBytes == 2) {
      // We have all the data we need for a 2-byte message
      MidiMessage msg;
      msg.status_ = midiStatus;
      msg.data1_ = midiData1;
      msg.data2_ = data;
      MidiInDevice::onDriverMessage(msg);

      // Reset data count but keep status for running status
      midiDataCount = 0;
    }
  }
}