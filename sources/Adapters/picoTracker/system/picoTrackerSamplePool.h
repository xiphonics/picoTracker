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

  void SaveSampleCacheForCurrentPool(const char *projectName) override;
  bool rebuildSampleFromCache(const SampleCacheEntry &e) override;
  void ResumeFromCache(uint32_t flashEraseOffset,
                       uint32_t flashWriteOffset) override {
    flashEraseOffset_ = flashEraseOffset;
    flashWriteOffset_ = flashWriteOffset;
    Trace::Log("SAMPLEPOOL", "Resumed flash allocator: erase=%u write=%u",
               flashEraseOffset_, flashWriteOffset_);
  }

  static uint32_t GetFlashEraseOffset() { return flashEraseOffset_; }
  static uint32_t GetFlashWriteOffset() { return flashWriteOffset_; }

  uint32_t GetSampleCacheBuildId() const override;

protected:
  virtual bool loadSample(const char *name);
  virtual bool unloadSample(uint32_t index);

private:
  bool LoadInFlash(WavFile *wave);

  static uint32_t flashEraseOffset_;
  static uint32_t flashWriteOffset_;
  static uint32_t flashLimit_;
};

#endif
