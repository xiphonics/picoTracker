/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "picoTrackerMidiInDevice.h"
#include "Adapters/picoTracker/platform/platform.h"
#include "Externals/etl/include/etl/queue_spsc_atomic.h"
#include "Services/Midi/MidiMessage.h"
#include "System/Console/Trace.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/sync.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"

// ETL queue for MIDI input
static etl::queue_spsc_atomic<uint8_t, 128> midi_rx_queue;

// Static pointer to the MIDI device instance
static picoTrackerMidiInDevice *g_midiInDevice = nullptr;

// UART interrupt handler - called directly from the IRQ
void __isr __time_critical_func(midi_uart_irq_handler)() {
  uint32_t status = uart_get_hw(MIDI_UART)->mis; // the reason this irq happened

  // Check if this is a UART RX interrupt
  if (status & UART_UARTMIS_RXMIS_BITS || status & UART_UARTMIS_RTMIS_BITS) {
    // Process all available data
    while (uart_is_readable(MIDI_UART)) {
      uint8_t data = uart_getc(MIDI_UART);
      // Store in ETL queue
      if (!midi_rx_queue.full()) {
        midi_rx_queue.push(data);
      } else {
        // MIDI RX Queue full!
        NAssert(false);
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
  // Clear the queue
  midi_rx_queue.clear();

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
  uint8_t data;
  // Process any data that the interrupt handler has placed in the queue
  while (midi_rx_queue.pop(data)) {
    processMidiData(data);
  }
}
