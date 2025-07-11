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
  //  uart_putc_raw(MIDI_UART, msg.status_);
  HAL_UART_Transmit(&MIDI_UART, (const uint8_t *)msg.status_, 1, 10);

  if (msg.status_ < 0xF0) {
    //    uart_putc_raw(MIDI_UART, msg.data1_);
    HAL_UART_Transmit(&MIDI_UART, (const uint8_t *)msg.data1_, 1, 10);
    if (msg.data2_ != MidiMessage::UNUSED_BYTE) {
      // uart_putc_raw(MIDI_UART, msg.data2_);
      HAL_UART_Transmit(&MIDI_UART, (const uint8_t *)msg.data2_, 1, 10);
    }
  }
}
