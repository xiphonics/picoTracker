/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "advUSBMidiOutDevice.h"
#include "Adapters/adv/platform/platform.h"
#include "Adapters/adv/usb/usb_utils.h"

advUSBMidiOutDevice::advUSBMidiOutDevice(const char *name)
    : MidiOutDevice(name) {}

bool advUSBMidiOutDevice::Init() { return true; }

void advUSBMidiOutDevice::Close(){};

bool advUSBMidiOutDevice::Start() { return true; };

void advUSBMidiOutDevice::Stop() {}

void advUSBMidiOutDevice::SendImmediate(MidiMessage &msg) { SendMessage(msg); }

void advUSBMidiOutDevice::SendMessage(MidiMessage &msg) {
  uint8_t midicmd[3] = {0, 0, 0};

  midicmd[0] = msg.status_;
  if (msg.status_ < 0xF0) {
    midicmd[1] = msg.data1_;
    midicmd[2] = msg.data2_;
    sendUSBMidiMessage(midicmd, 3);
  } else {
    sendUSBMidiMessage(midicmd, 1);
  }
}
