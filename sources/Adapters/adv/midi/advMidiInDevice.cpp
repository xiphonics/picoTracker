/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "advMidiInDevice.h"
#include "Adapters/adv/platform/platform.h"
#include "Services/Midi/MidiMessage.h"
#include "System/Console/Trace.h"
#include "main.h"

// Ring buffer size for MIDI input
#define MIDI_UART_BUFFER_SIZE 128

// Static pointer to the UART handle for use in the callback
static UART_HandleTypeDef *g_midi_huart = &MIDI_UART;

// Buffer for the HAL to store the single received byte
static uint8_t g_rx_byte;

// Static pointer to the MIDI device instance
static advMidiInDevice *g_midiInDevice = nullptr;

// Ring buffer for MIDI input
static uint8_t midi_rx_buffer[MIDI_UART_BUFFER_SIZE];
static volatile uint32_t midi_rx_head = 0;
static volatile uint32_t midi_rx_tail = 0;

// --- STM32 HAL UART Receive Complete Callback ---
// This function is called by the HAL driver's ISR when a byte is received.
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  // Check if the interrupt is from our MIDI UART
  if (huart->Instance == g_midi_huart->Instance) {
    // Store the received byte in the ring buffer
    uint32_t next_head = (midi_rx_head + 1) % MIDI_UART_BUFFER_SIZE;
    if (next_head != midi_rx_tail) {
      midi_rx_buffer[midi_rx_head] = g_rx_byte;
      midi_rx_head = next_head;
    }
    // IMPORTANT: Re-arm the UART interrupt to receive the next byte
    HAL_UART_Receive_IT(g_midi_huart, &g_rx_byte, 1);
  }
}

advMidiInDevice::advMidiInDevice(const char *name) : MidiInDevice(name) {
  g_midiInDevice = this;
  Trace::Log("MIDI", "Created MIDI input device %s", name);
}

advMidiInDevice::~advMidiInDevice() {
  g_midiInDevice = nullptr;
  closeDriver();
}

bool advMidiInDevice::initDriver() {
  // UART should already be initialized by STM32CubeMX in main.c
  if (g_midi_huart == nullptr) {
    Trace::Log("MIDI", "ERROR: MIDI UART handle is not initialized!");
    return false;
  }
  Trace::Log("MIDI", "Initialized MIDI input driver");
  return true;
}

void advMidiInDevice::closeDriver() {
  // Stop the UART receive interrupt
  if (g_midi_huart != nullptr) {
    HAL_UART_AbortReceive_IT(g_midi_huart);
  }
  Trace::Log("MIDI", "Closed MIDI input driver");
}

bool advMidiInDevice::Start() { return startDriver(); }

void advMidiInDevice::Stop() { stopDriver(); }

bool advMidiInDevice::startDriver() {
  // Reset the ring buffer
  midi_rx_head = 0;
  midi_rx_tail = 0;

  // Start the UART receive in interrupt mode.
  // The HAL will now listen for 1 byte. When it arrives, it will call
  // HAL_UART_RxCpltCallback, where we re-arm it to continue listening.
  HAL_StatusTypeDef status = HAL_UART_Receive_IT(g_midi_huart, &g_rx_byte, 1);

  if (status != HAL_OK) {
    Trace::Log("MIDI", "Failed to start MIDI input driver, HAL_ERROR: %d",
               status);
    return false;
  }

  Trace::Log("MIDI", "Started MIDI input driver");
  return true;
}

void advMidiInDevice::stopDriver() {
  // Stop the UART receive interrupt
  if (g_midi_huart != nullptr) {
    HAL_UART_AbortReceive_IT(g_midi_huart);
  }
  Trace::Log("MIDI", "Stopped MIDI input driver");
}

void advMidiInDevice::poll() {
  // processes any data that the HAL_UART_RxCpltCallback has placed in the
  // ring buffer.
  while (true) {
    // Use a critical section to safely read head and tail on
    // multicore/preemptive systems For STM32, this prevents an interrupt from
    // modifying midi_rx_head while we read it.
    uint32_t primask_state = __get_PRIMASK();
    __disable_irq();

    if (midi_rx_head == midi_rx_tail) {
      __set_PRIMASK(primask_state); // Re-enable interrupts
      break;
    }

    // Get data and update tail
    auto data = midi_rx_buffer[midi_rx_tail];
    midi_rx_tail = (midi_rx_tail + 1) % MIDI_UART_BUFFER_SIZE;

    __set_PRIMASK(primask_state); // Re-enable interrupts

    // Process the MIDI data
    processMidiData(data);
  }
}
