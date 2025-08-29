/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "advMidiInDevice.h"
#include "Adapters/adv/platform/platform.h"
#include "Externals/etl/include/etl/queue_spsc_atomic.h"
#include "Services/Midi/MidiMessage.h"
#include "System/Console/Trace.h"
#include "main.h"

// ETL queue for MIDI input
static etl::queue_spsc_atomic<uint8_t, 128> midi_rx_queue;

// Static pointer to the UART handle for use in the callback
static UART_HandleTypeDef *g_midi_huart = &MIDI_UART;

// Buffer for the HAL to store the single received byte
static uint8_t g_rx_byte;

// Static pointer to the MIDI device instance
static advMidiInDevice *g_midiInDevice = nullptr;

// --- STM32 HAL UART Receive Complete Callback ---
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  // Check if the interrupt is from our MIDI UART
  if (huart->Instance == g_midi_huart->Instance) {
    // Store the received byte in the ETL queue
    if (!midi_rx_queue.full()) {
      midi_rx_queue.push(g_rx_byte);
    } else {
      // MIDI RX Queue full!
      NAssert(false);
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
  // Clear the queue
  midi_rx_queue.clear();

  // Start the UART receive in interrupt mode.
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
  uint8_t data;
  // processes any data that the HAL_UART_RxCpltCallback has placed in the queue
  while (midi_rx_queue.pop(data)) {
    // Process the MIDI data
    processMidiData(data);
  }
}