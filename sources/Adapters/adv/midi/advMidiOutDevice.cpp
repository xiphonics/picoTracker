/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "advMidiOutDevice.h"
#include "System/Console/Trace.h"
#include "main.h"
#include "stm32h7xx_hal.h"
#include "usart.h"

advMidiOutDevice::advMidiOutDevice(const char *name) : MidiOutDevice(name) {}

bool advMidiOutDevice::Init() { return true; }

void advMidiOutDevice::Close(){};

bool advMidiOutDevice::Start() { return true; };

void advMidiOutDevice::Stop() {}

void advMidiOutDevice::SendMessage(MidiMessage &msg) {
  uint8_t status = msg.status_;
  HAL_UART_Transmit(&MIDI_UART, &status, 1, 10);

  if (msg.status_ < 0xF0) {
    uint8_t data1 = msg.data1_;
    HAL_UART_Transmit(&MIDI_UART, &data1, 1, 10);
    if (msg.data2_ != MidiMessage::UNUSED_BYTE) {
      uint8_t data2 = msg.data2_;
      HAL_UART_Transmit(&MIDI_UART, &data2, 1, 10);
    }
  }
}
