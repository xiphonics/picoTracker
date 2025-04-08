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

  // Reset the ring buffer
  midi_rx_head = 0;
  midi_rx_tail = 0;

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
  // Check if there's any data to process
  // Process any data in the ring buffer
  while (true) {
    if (midi_rx_head == midi_rx_tail) {
      break;
    }
    // Get data and update tail
    auto data = midi_rx_buffer[midi_rx_tail];
    midi_rx_tail = (midi_rx_tail + 1) % MIDI_UART_BUFFER_SIZE;
    // Process the MIDI data
    // Trace::Debug("Processing byte: 0x%02X", data[j]);
    processMidiData(data);
  }
}
