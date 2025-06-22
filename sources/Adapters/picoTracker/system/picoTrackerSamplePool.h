/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _PICOTRACKERSAMPLEPOOL_H_
#define _PICOTRACKERSAMPLEPOOL_H_
#include "Application/Instruments/SamplePool.h"
#include "Application/Instruments/WavFile.h"
#include "System/Console/Trace.h"

class picoTrackerSamplePool : public SamplePool {
public:
  picoTrackerSamplePool();
  virtual void Reset();
  ~picoTrackerSamplePool() {}
  virtual bool CheckSampleFits(int sampleSize);

  virtual uint32_t GetAvailableSampleStorageSpace() override {
    return (flashLimit_ - flashWriteOffset_);
  }

protected:
  virtual bool loadSample(const char *name);
  virtual bool unloadSample();

private:
  bool LoadInFlash(WavFile *wave);

  static uint32_t flashEraseOffset_;
  static uint32_t flashWriteOffset_;
  static uint32_t flashLimit_;
};

#endif
