/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "advMidiInDevice.h"
#include "Services/Midi/MidiMessage.h"
#include "System/Console/Trace.h"

// Ring buffer size for MIDI input
#define MIDI_UART_BUFFER_SIZE 128

// Static pointer to the MIDI device instance
static advMidiInDevice *g_midiInDevice = nullptr;

// Ring buffer for MIDI input
static uint8_t midi_rx_buffer[MIDI_UART_BUFFER_SIZE];
static volatile uint32_t midi_rx_head = 0;
static volatile uint32_t midi_rx_tail = 0;

advMidiInDevice::advMidiInDevice(const char *name) : MidiInDevice(name) {
  g_midiInDevice = this;
  Trace::Log("MIDI", "Created Advance MIDI input device %s", name);
}

advMidiInDevice::~advMidiInDevice() {
  g_midiInDevice = nullptr;
  closeDriver();
}

bool advMidiInDevice::initDriver() {
  // UART should already be initialized in platform.cpp
  Trace::Log("MIDI", "Initialized MIDI input driver");
  return true;
}

void advMidiInDevice::closeDriver() {
  // Disable UART RX interrupt
  // irq_set_enabled(MIDI_UART_IRQ, false);
  // uart_set_irq_enables(MIDI_UART, false, false);
  Trace::Log("MIDI", "Closed MIDI input driver");
}

bool advMidiInDevice::Start() { return startDriver(); }

void advMidiInDevice::Stop() { stopDriver(); }

bool advMidiInDevice::startDriver() {

  // Reset the ring buffer
  midi_rx_head = 0;
  midi_rx_tail = 0;

  // Set up the interrupt handler
  // irq_set_exclusive_handler(MIDI_UART_IRQ, midi_uart_irq_handler);
  // irq_set_enabled(MIDI_UART_IRQ, true);

  // Enable UART RX interrupt
  // uart_set_irq_enables(MIDI_UART, true, false);

  // Trace::Log("MIDI", "Started MIDI input driver on UART %d (IRQ: %d, Baud:
  // %d)",
  //            MIDI_UART == uart0 ? 0 : 1, MIDI_UART_IRQ, MIDI_BAUD_RATE);
  return true;
}

void advMidiInDevice::stopDriver() {
  // Disable UART RX interrupt
  // irq_set_enabled(MIDI_UART_IRQ, false);
  // uart_set_irq_enables(MIDI_UART, false, false);
  Trace::Log("MIDI", "Stopped MIDI input driver");
}

void advMidiInDevice::poll() {
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
