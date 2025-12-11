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

advMidiService::~advMidiService(){};

void advMidiService::OnPlayerStart() {
  // Keep base behavior (device restart + queued transport) then send
  // immediately
  MidiService::OnPlayerStart();

  Config *config = Config::GetInstance();
  const bool midiSyncEnabled =
      config ? (config->GetValue("MIDISYNC") > 0) : false;
  if (!midiSyncEnabled) {
    return;
  }

  MidiMessage msg;
  msg.status_ = MidiMessage::MIDI_START;
  etl::vector<MidiMessage, MIDI_MAX_MESG_QUEUE> tmp;
  tmp.emplace_back(msg.status_, msg.data1_, msg.data2_);
  midiOutDevice_.SendQueue(tmp);
  usbMidiOutDevice_.SendQueue(tmp);
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
  etl::vector<MidiMessage, MIDI_MAX_MESG_QUEUE> tmp;
  tmp.emplace_back(msg.status_, msg.data1_, msg.data2_);
  midiOutDevice_.SendQueue(tmp);
  usbMidiOutDevice_.SendQueue(tmp);
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
