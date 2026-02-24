/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

class SoundSource;

struct BundledWavetableInfo {
  const char *name;
  const uint8_t *data;
  uint16_t frameCount;
  uint16_t samplesPerFrame;
};

extern const BundledWavetableInfo gBundledWavetables[];
extern const size_t gBundledWavetableCount;
SoundSource *GetBundledWavetableSource(int index);
