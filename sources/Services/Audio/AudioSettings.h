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

#include "config/StringLimits.h"
#include "Externals/etl/include/etl/string.h"
// Used to propagate audio hints & settings

struct AudioSettings {
  etl::string<STRING_AUDIO_API_MAX> audioAPI_;
  etl::string<STRING_AUDIO_DEVICE_MAX> audioDevice_;
  int bufferSize_;
  int preBufferCount_;
};

#endif
