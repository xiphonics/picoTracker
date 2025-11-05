/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "AudioOut.h"
#include "Application/Player/SyncMaster.h"

AudioOut::AudioOut() : AudioMixer("AudioOut"), sampleOffset_(0){};

AudioOut::~AudioOut(){};

int AudioOut::getPlaySampleCount() {
  sampleOffset_ += SyncMaster::instance().GetPlaySampleCount();
  int count = int(sampleOffset_);
  sampleOffset_ -= count;
  return count;
};
