/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "MidiInMerger.h"
#include "Services/Controllers/MultiChannelAdapter.h"

MidiInMerger::MidiInMerger() : ControllerSource("midi", "all"){};

MidiInMerger::~MidiInMerger(){};

Channel *MidiInMerger::GetChannel(const char *name) {
  // First let's see if the map contains it
  std::string key(name);
  tChannelMap::iterator i = channels_.find(key);
  if (i != channels_.end()) {
    return i->second;
  }
  // The map doesn't contain it so we need to create one

  MultiChannelAdapter *newC = 0;
  for (Begin(); !IsDone(); Next()) {
    MidiInDevice &current = CurrentItem();
    Channel *c = current.GetChannel(name);
    if (c) {
      if (!newC) {
        newC = new MultiChannelAdapter(name);
      }
      c->AddObserver(*newC);
    }
  }
  if (newC) {
    channels_[key] = newC;
  };
  return newC;
};
