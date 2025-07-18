/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "advSamplePool.h"

// TODO: Only using sampleStore1 for now
__attribute__((section(".SDRAM1"))) __attribute__((aligned(32)))
uint8_t sampleStore1[STORE1_SIZE];
__attribute__((section(".SDRAM2"))) __attribute__((aligned(32)))
uint8_t sampleStore2[STORE2_SIZE];

uint32_t advSamplePool::writeOffset1_ = 0;
uint32_t advSamplePool::writeOffset2_ = 0;
uint32_t advSamplePool::storeLimit1_ = STORE1_SIZE;
uint32_t advSamplePool::storeLimit2_ = STORE2_SIZE;

advSamplePool::advSamplePool() : SamplePool() {}

void advSamplePool::Reset() {
  count_ = 0;
  for (int i = 0; i < MAX_SAMPLES; i++) {
    SAFE_DELETE(wav_[i]);
    SAFE_FREE(names_[i]);
  };

  volatile void *dummy = &sampleStore2;
  // Reset flash erase and write pointers when we close project
  writeOffset1_ = 0;
  writeOffset2_ = 0;
};

bool advSamplePool::CheckSampleFits(int sampleSize) {
  return (writeOffset1_ + sampleSize) <= storeLimit1_;
}

uint32_t advSamplePool::GetAvailableSampleStorageSpace() {
  return storeLimit1_ - writeOffset1_;
}

bool advSamplePool::loadSample(const char *name) {
  Trace::Log("SAMPLEPOOL", "Loading sample into flash: %s", name);

  if (count_ == MAX_SAMPLES)
    return false;

  WavFile *wave = WavFile::Open(name);
  if (wave) {
    wav_[count_] = wave;
    names_[count_] = (char *)SYS_MALLOC(strlen(name) + 1);
    strcpy(names_[count_], name);
    count_++;
    Load(wave);
    wave->Close();

    return true;
  } else {
    Trace::Error("Failed to load sample:%s", name);
    return false;
  }
};

bool advSamplePool::Load(WavFile *wave) {

  uint32_t fileSize = wave->GetDiskSize(-1);
  Trace::Debug("File size: %i", fileSize);

  if (writeOffset1_ + fileSize > storeLimit1_) {
    Trace::Error("Sample doesn't fit in available sample storage (need: %i - "
                 "avail: %i) ",
                 fileSize, storeLimit1_ - writeOffset1_);
    return false;
  }

  // Set wave base
  wave->SetSampleBuffer((short *)(sampleStore1 + writeOffset1_));

  uint32_t offset = 0;
  uint32_t br = 0;

  wave->Rewind();
  wave->Read(sampleStore1 + writeOffset1_, BUFFER_SIZE, &br);
  while (br > 0) {
    // Trace::Debug("Wrote %i bytes", br);
    writeOffset1_ += br;
    // Ensure 4-byte alignment for SDRAM access
    writeOffset1_ = (writeOffset1_ + 3) & ~3;
    wave->Read(sampleStore1 + writeOffset1_, BUFFER_SIZE, &br);
  }
  return true;
};

bool advSamplePool::unloadSample() { return false; };
