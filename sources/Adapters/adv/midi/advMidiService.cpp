/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "advMidiService.h"
#include "advMidiInDevice.h"
#include "advMidiOutDevice.h"
#include "advUSBMidiOutDevice.h"

advMidiService::advMidiService()
    : // Initialize static member variables with their respective names
      midiOutDevice_("MIDI OUT"), usbMidiOutDevice_("USB"),
      midiInDevice_("MIDI IN") /*, usbMidiInDevice_("USB MIDI IN") */ {
  // Add MIDI output devices to the output device list
  outList_.insert(outList_.end(), &midiOutDevice_);
  outList_.insert(outList_.end(), &usbMidiOutDevice_);

  // Add MIDI input devices to the input device list
  inList_.insert(inList_.end(), &midiInDevice_);
  // inList_.insert(inList_.end(), &usbMidiInDevice_);
};

advMidiService::~advMidiService() {};

void advMidiService::OnPlayerStart() { MidiService::OnPlayerStart(); }

void advMidiService::OnPlayerStop() { MidiService::OnPlayerStop(); }

void advMidiService::poll() {
  // Poll all MIDI input devices
  for (auto dev : inList_) {
    advMidiInDevice *ptDev = (advMidiInDevice *)dev;
    if (ptDev) {
      ptDev->poll();
    }
  }
};
