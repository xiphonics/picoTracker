/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "advSamplePool.h"

__attribute__((section(".SDRAM2"))) __attribute__((aligned(32)))
uint8_t sampleStore1[STORE1_SIZE];
__attribute__((section(".SDRAM1"))) __attribute__((aligned(32)))
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
  return ((writeOffset1_ + sampleSize) <= storeLimit1_) ||
         ((writeOffset2_ + sampleSize) <= storeLimit2_);
}

uint32_t advSamplePool::GetAvailableSampleStorageSpace() {
  return storeLimit1_ - writeOffset1_ + storeLimit2_ - writeOffset2_;
}

bool advSamplePool::loadSample(const char *name) {
  Trace::Log("SAMPLEPOOL", "Loading sample into flash: %s", name);

  if (count_ == MAX_SAMPLES)
    return false;

  //  WavFile *wave = WavFile::Open(name);
  auto wave = WavFile::Open(name);
  if (!wave) {
    Trace::Error("Failed to load sample:%s", name);
    return false;
  }

  wav_[count_] = wave.value();
  names_[count_] = (char *)SYS_MALLOC(strlen(name) + 1);
  strcpy(names_[count_], name);
  count_++;
  Load(wave.value());
  wave.value()->Close();

  return true;
};

bool advSamplePool::Load(WavFile *wave) {

  uint32_t fileSize = wave->GetDiskSize(-1);
  Trace::Debug("File size: %i", fileSize);

  // Select the sample pool with the least space where it will fit in order to
  // leave the biggest free space for any potential bigger future samples
  uint32_t *writeOffset = 0;
  uint8_t *sampleStore;

  if ((writeOffset1_ + fileSize < storeLimit1_) &&
      (writeOffset2_ + fileSize < storeLimit2_)) {
    // fits both
    if ((storeLimit1_ - writeOffset1_) < (storeLimit2_ - writeOffset2_)) {
      writeOffset = &writeOffset1_;
      sampleStore = sampleStore1;
    } else {
      writeOffset = &writeOffset2_;
      sampleStore = sampleStore2;
    }
  }
  // Only one of them can potentially fit the sample
  else if (writeOffset1_ + fileSize < storeLimit1_) {
    writeOffset = &writeOffset1_;
    sampleStore = sampleStore1;
  } else if (writeOffset2_ + fileSize < storeLimit2_) {
    writeOffset = &writeOffset2_;
    sampleStore = sampleStore2;
  } else {
    Trace::Error("Sample doesn't fit in available sample storage (need: %i - "
                 "avail: %i) ",
                 fileSize, storeLimit1_ - writeOffset1_);
    return false;
  }

  // Ensure 4-byte alignment for SDRAM access at start of each file
  *writeOffset = (*writeOffset + 3) & ~3;

  // Set wave base
  wave->SetSampleBuffer((short *)(sampleStore + *writeOffset));

  uint32_t offset = 0;
  uint32_t br = 0;

  wave->Rewind();
  wave->Read(sampleStore + *writeOffset, BUFFER_SIZE, &br);
  while (br > 0) {
    // Trace::Debug("Wrote %i bytes", br);
    *writeOffset += br;
    wave->Read(sampleStore + *writeOffset, BUFFER_SIZE, &br);
  }
  return true;
};

bool advSamplePool::unloadSample(int index) {
  // Will only optimize samples within the sample pool where the sample resides
  // TODO (democloid): potentially optimize across sample pools
  if (index < 0 || index >= count_)
    return false;

  std::free(names_[index]);
  names_[index] = nullptr;

  // Get information about how we should do the memmove
  uint32_t deletedSize = ((WavFile *)wav_[index])->GetDiskSize(0);
  uint32_t bufferSize = 0;
  void *moveDst = wav_[index]->GetSampleBuffer(0);
  void *moveSrc = nullptr;
  if (index < count_ - 1) {
    moveSrc = wav_[index + 1]->GetSampleBuffer(0);
  }

  // Update each SoundSource
  for (int j = index; j < count_ - 1; ++j) {
    void *dstBuffer = wav_[j]->GetSampleBuffer(0);
    wav_[j] = wav_[j + 1];
    names_[j] = names_[j + 1];
    ((WavFile *)wav_[j])->SetSampleBuffer(static_cast<short *>(dstBuffer));
    // this gives us the total size to be moved
    bufferSize += ((WavFile *)wav_[j])->GetDiskSize(0);
  }

  // correct the write pointer (depends on pool)
  if (moveDst >= sampleStore1 && moveDst < sampleStore1 + STORE1_SIZE) {
    writeOffset1_ -= deletedSize;
  } else if (moveDst >= sampleStore2 && moveDst < sampleStore2 + STORE2_SIZE) {
    writeOffset2_ -= deletedSize;
  } else {
    // should never get here
    Trace::Error("Invalid sample address while deleting");
    return false;
  }

  // If there was anything to move (i.e: it's not the last sample) we do so
  // now in a sigle chunk
  if (bufferSize > 0) {
    std::memmove(moveDst, moveSrc, bufferSize);
  }

  // clear the last slot (now unused)
  wav_[count_ - 1] = nullptr;
  names_[count_ - 1] = nullptr;

  // decrement sample count
  --count_;

  return true;
}
