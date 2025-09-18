/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#ifndef _ADVSAMPLEPOOL_H_
#define _ADVSAMPLEPOOL_H_
#include "Application/Instruments/SamplePool.h"
#include "Application/Instruments/WavFile.h"
#include "System/Console/Trace.h"

#define STORE1_SIZE 32 * 1024 * 1024
#define STORE2_SIZE 16 * 1024 * 1024

extern uint8_t sampleStore1[];
extern uint8_t sampleStore2[];

class advSamplePool : public SamplePool {
public:
  advSamplePool();
  virtual void Reset();
  ~advSamplePool() {}
  virtual bool CheckSampleFits(int sampleSize);
  virtual uint32_t GetAvailableSampleStorageSpace();

protected:
  virtual bool loadSample(const char *name);
  virtual bool unloadSample(int i);

private:
  bool Load(WavFile *wave);

  static uint32_t writeOffset1_;
  static uint32_t storeLimit1_;
  static uint32_t writeOffset2_;
  static uint32_t storeLimit2_;
};

#endif
