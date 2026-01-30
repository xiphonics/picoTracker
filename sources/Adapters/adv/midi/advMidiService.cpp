/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "advMidiService.h"
#include "Application/Model/Config.h"
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

void advMidiService::OnPlayerStart() {
  // TODO (democloid): this is a hack. We need to understand the order of
  // operations of MIDI messages and why the transport messages get lost,
  // presumably due to Services/Midi/MidiService.cpp::AdvancePlayQueue clearing
  // it before it's send out
  MidiService::OnPlayerStart();

  Config *config = Config::GetInstance();
  const bool midiSyncEnabled =
      config ? (config->GetValue("MIDISYNC") > 0) : false;
  if (!midiSyncEnabled) {
    return;
  }

  MidiMessage msg;
  msg.status_ = MidiMessage::MIDI_START;
  midiOutDevice_.SendImmediate(msg);
  usbMidiOutDevice_.SendImmediate(msg);
}

void advMidiService::OnPlayerStop() {
  MidiService::OnPlayerStop();

  Config *config = Config::GetInstance();
  const bool midiSyncEnabled =
      config ? (config->GetValue("MIDISYNC") > 0) : false;
  if (!midiSyncEnabled) {
    return;
  }

  MidiMessage msg;
  msg.status_ = MidiMessage::MIDI_STOP;
  midiOutDevice_.SendImmediate(msg);
  usbMidiOutDevice_.SendImmediate(msg);
}

void advMidiService::poll() {
  // Poll all MIDI input devices
  for (auto dev : inList_) {
    advMidiInDevice *ptDev = (advMidiInDevice *)dev;
    if (ptDev) {
      ptDev->poll();
    }
  }
};
