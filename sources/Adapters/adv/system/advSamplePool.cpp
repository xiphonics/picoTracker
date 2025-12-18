/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "advSamplePool.h"

__attribute__((section(".SDRAM2")))
__attribute__((aligned(32))) uint8_t sampleStore1[STORE1_SIZE];
__attribute__((section(".SDRAM1")))
__attribute__((aligned(32))) uint8_t sampleStore2[STORE2_SIZE];

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
  Trace::Log("SAMPLEPOOL", "Loading sample into ram: %s", name);

  if (count_ == MAX_SAMPLES)
    return false;

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

bool advSamplePool::unloadSample(uint32_t index) {
  // Will only optimize samples within the sample pool where the sample resides
  // TODO (democloid): potentially optimize across sample pools
  if (index < 0 || index >= count_)
    return false;

  // Gather memory layout info for the sample being removed
  void *moveDst = wav_[index] ? wav_[index]->GetSampleBuffer(0) : nullptr;
  if (!moveDst) {
    Trace::Error("Invalid sample buffer while deleting");
    return false;
  }

  uint8_t *poolBase = nullptr;
  uint32_t poolLimit = 0;
  uint32_t *poolWriteOffset = nullptr;
  if (moveDst >= sampleStore1 && moveDst < sampleStore1 + STORE1_SIZE) {
    poolBase = sampleStore1;
    poolLimit = STORE1_SIZE;
    poolWriteOffset = &writeOffset1_;
  } else if (moveDst >= sampleStore2 && moveDst < sampleStore2 + STORE2_SIZE) {
    poolBase = sampleStore2;
    poolLimit = STORE2_SIZE;
    poolWriteOffset = &writeOffset2_;
  } else {
    Trace::Error("Invalid sample address while deleting");
    return false;
  }

  // Find the first subsequent sample that resides in the same pool so we only
  // compact that pool.
  uint8_t *moveSrc = nullptr;
  for (uint32_t j = index + 1; j < count_; ++j) {
    uint8_t *candidate =
        wav_[j] ? static_cast<uint8_t *>(wav_[j]->GetSampleBuffer(0)) : nullptr;
    if (candidate && candidate >= poolBase &&
        candidate < poolBase + poolLimit) {
      moveSrc = candidate;
      break;
    }
  }

  // Free deleted entry resources
  SAFE_DELETE(wav_[index]);
  SAFE_FREE(names_[index]);

  uint32_t shift = 0;
  uint32_t bytesToMove = 0;
  if (moveSrc != nullptr) {
    shift = static_cast<uint32_t>(moveSrc - static_cast<uint8_t *>(moveDst));
    bytesToMove = *poolWriteOffset - static_cast<uint32_t>(moveSrc - poolBase);
  } else {
    // Removing the last sample in this pool: reclaim the tail of the pool
    shift = *poolWriteOffset -
            static_cast<uint32_t>(static_cast<uint8_t *>(moveDst) - poolBase);
  }

  // Shift wav/name pointers down in the arrays
  for (uint32_t j = index; j < count_ - 1; ++j) {
    wav_[j] = wav_[j + 1];
    names_[j] = names_[j + 1];
  }

  // If there was anything to move (i.e: there's a later sample in the same
  // pool) shift the pool data in one chunk
  if (bytesToMove > 0) {
    std::memmove(moveDst, moveSrc, bytesToMove);
  }

  // Update pool write offset by the bytes we freed
  *poolWriteOffset -= shift;

  // Update each SoundSource in this pool to point at the new buffer location
  if (shift > 0) {
    for (uint32_t j = index; j < count_ - 1; ++j) {
      uint8_t *buf = wav_[j]
                         ? static_cast<uint8_t *>(wav_[j]->GetSampleBuffer(0))
                         : nullptr;
      if (buf && buf >= poolBase && buf < poolBase + poolLimit) {
        ((WavFile *)wav_[j])
            ->SetSampleBuffer(reinterpret_cast<short *>(buf - shift));
      }
    }
  }

  // clear the last slot (now unused)
  wav_[count_ - 1] = nullptr;
  names_[count_ - 1] = nullptr;

  // decrement sample count
  --count_;

  // notify observers so sample variables can adjust their indexes
  SetChanged();
  SamplePoolEvent ev;
  ev.index_ = index;
  ev.type_ = SPET_DELETE;
  NotifyObservers(&ev);

  return true;
}
