/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "Application/Instruments/Wavetables/BundledWavetables.h"
#include "Application/Instruments/SoundSource.h"

#include "Application/Instruments/Wavetables/PPG/PpgWavetable00.h"
#include "Application/Instruments/Wavetables/PPG/PpgWavetable01.h"
#include "Application/Instruments/Wavetables/PPG/PpgWavetable02.h"
#include "Application/Instruments/Wavetables/PPG/PpgWavetable03.h"

namespace {

static constexpr uint16_t kPpgFrameCount = 64;
static constexpr uint16_t kPpgSamplesPerFrame = 128;

class BundledWavetableSource : public SoundSource {
public:
  BundledWavetableSource(const uint8_t *data, uint16_t frameCount,
                        uint16_t samplesPerFrame)
      : data_(data), frameCount_(frameCount), samplesPerFrame_(samplesPerFrame) {}

  void *GetSampleBuffer(int note) override { (void)note; return const_cast<uint8_t *>(data_); }
  int GetSize(int note) override { (void)note; return static_cast<int>(frameCount_) * static_cast<int>(samplesPerFrame_); }
  int GetSampleRate(int note) override { (void)note; return 44100; }
  int GetChannelCount(int note) override { (void)note; return 1; }
  bool IsMulti() override { return false; }
  int GetRootNote(int note) override { (void)note; return 60; }
  float GetLengthInSec() override { return static_cast<float>(GetSize(0)) / static_cast<float>(GetSampleRate(0)); }

private:
  const uint8_t *data_;
  uint16_t frameCount_;
  uint16_t samplesPerFrame_;
};

static BundledWavetableSource ppgTable00Source(ppgTable00Data, kPpgFrameCount, kPpgSamplesPerFrame);
static BundledWavetableSource ppgTable01Source(ppgTable01Data, kPpgFrameCount, kPpgSamplesPerFrame);
static BundledWavetableSource ppgTable02Source(ppgTable02Data, kPpgFrameCount, kPpgSamplesPerFrame);
static BundledWavetableSource ppgTable03Source(ppgTable03Data, kPpgFrameCount, kPpgSamplesPerFrame);

static BundledWavetableSource *gBundledPpgSources[] = {
    &ppgTable00Source,
    &ppgTable01Source,
    &ppgTable02Source,
    &ppgTable03Source,
};

} // namespace

const BundledWavetableInfo gBundledWavetables[] = {
    {"PPG WT 00", ppgTable00Data, kPpgFrameCount, kPpgSamplesPerFrame},
    {"PPG WT 01", ppgTable01Data, kPpgFrameCount, kPpgSamplesPerFrame},
    {"PPG WT 02", ppgTable02Data, kPpgFrameCount, kPpgSamplesPerFrame},
    {"PPG WT 03", ppgTable03Data, kPpgFrameCount, kPpgSamplesPerFrame},
};

const size_t gBundledWavetableCount =
    sizeof(gBundledWavetables) / sizeof(gBundledWavetables[0]);

SoundSource *GetBundledWavetableSource(int index) {
  if (gBundledWavetableCount == 0) { return nullptr; }
  if (index < 0) { index = 0; }
  if (index >= static_cast<int>(gBundledWavetableCount)) {
    index = static_cast<int>(gBundledWavetableCount) - 1;
  }
  return gBundledPpgSources[index];
}
