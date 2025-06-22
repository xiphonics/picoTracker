/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "Mixer.h"

Mixer::Mixer() : Persistent("MIXER") { Clear(); };

Mixer::~Mixer(){};

void Mixer::Clear() {

  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
    channelBus_[i] = i;
  }
};

void Mixer::SaveContent(tinyxml2::XMLPrinter *printer){};

void Mixer::RestoreContent(PersistencyDocument *doc) {}
