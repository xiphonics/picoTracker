/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _AUDIO_SETTINGS_H_
#define _AUDIO_SETTINGS_H_

#include <string>
// Used to propagate audio hints & settings

struct AudioSettings {
  std::string audioAPI_;
  std::string audioDevice_;
  int bufferSize_;
  int preBufferCount_;
};

#endif
