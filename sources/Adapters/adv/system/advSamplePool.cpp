/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "advSamplePool.h"
#include <cstring>
#include <utility>

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
    wav_[i].Close();
    nameStore_[i][0] = '\0';
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

  auto res = wav_[count_].Open(name);
  if (!res) {
    Trace::Error("Failed to load sample:%s", name);
    return false;
  }

  strncpy(nameStore_[count_], name, MAX_INSTRUMENT_FILENAME_LENGTH);
  nameStore_[count_][MAX_INSTRUMENT_FILENAME_LENGTH] = '\0';
  count_++;
  Load(wav_[count_ - 1]);
  wav_[count_ - 1].Close();

  return true;
};

bool advSamplePool::Load(WavFile &wave) {

  uint32_t fileSize = wave.GetDiskSize(-1);
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
  wave.SetSampleBuffer((short *)(sampleStore + *writeOffset));

  uint32_t offset = 0;
  uint32_t br = 0;

  // 10 updates per sample
  uint32_t writeStep = (9 + fileSize) / 10; // ceil to avoid 11 steps
  uint32_t totalWritten = 0;
  uint32_t subStep = 0;

  wave.Rewind();
  wave.Read(sampleStore + *writeOffset, BUFFER_SIZE, &br);

  while (br > 0) {
    totalWritten += br;

    while (totalWritten >= writeStep) {
      totalWritten -= writeStep;
      subStep++;
      updateStatus(importIndex * 10 + subStep, importCount * 10, "Importing");
    }

    // Trace::Debug("Wrote %i bytes", br);
    *writeOffset += br;
    wave.Read(sampleStore + *writeOffset, BUFFER_SIZE, &br);
  }
  return true;
};

bool advSamplePool::unloadSample(uint32_t index) {
  // Will only optimize samples within the sample pool where the sample resides
  // TODO (democloid): potentially optimize across sample pools
  if (index >= count_)
    return false;

  // Gather memory layout info for the sample being removed
  auto *moveDst = static_cast<uint8_t *>(wav_[index].GetSampleBuffer(0));
  if (moveDst == nullptr) {
    Trace::Error("Invalid sample buffer while deleting");
    return false;
  }

  auto *poolBase = static_cast<uint8_t *>(nullptr);
  uint32_t poolLimit = 0;
  uint32_t *poolWriteOffset = nullptr;
  if (moveDst >= sampleStore1 && moveDst < sampleStore1 + storeLimit1_) {
    poolBase = sampleStore1;
    poolLimit = storeLimit1_;
    poolWriteOffset = &writeOffset1_;
  } else if (moveDst >= sampleStore2 && moveDst < sampleStore2 + storeLimit2_) {
    poolBase = sampleStore2;
    poolLimit = storeLimit2_;
    poolWriteOffset = &writeOffset2_;
  } else {
    Trace::Error("Invalid sample address while deleting");
    return false;
  }

  const uint32_t moveDstOffset = static_cast<uint32_t>(moveDst - poolBase);
  if (moveDstOffset >= *poolWriteOffset) {
    Trace::Error("Invalid sample address while deleting (outside pool usage)");
    return false;
  }

  // Find the next sample in memory (not by index order) that resides in the
  // same pool so we only compact that pool.
  uint8_t *moveSrc = nullptr;
  for (uint32_t j = 0; j < count_; ++j) {
    if (j == index) {
      continue;
    }
    auto *candidate = static_cast<uint8_t *>(wav_[j].GetSampleBuffer(0));
    if (candidate == nullptr) {
      continue;
    }
    if (candidate >= poolBase && candidate < poolBase + poolLimit &&
        candidate > moveDst) {
      if (moveSrc == nullptr || candidate < moveSrc) {
        moveSrc = candidate;
      }
    }
  }

  uint32_t shift = 0;
  uint32_t bytesToMove = 0;
  if (moveSrc != nullptr) {
    shift = static_cast<uint32_t>(moveSrc - moveDst);
    bytesToMove = *poolWriteOffset - static_cast<uint32_t>(moveSrc - poolBase);
  } else {
    // Removing the last sample in this pool: reclaim the tail of the pool
    shift = *poolWriteOffset - moveDstOffset;
  }

  // If there was anything to move (i.e: there's a later sample in the same
  // pool) shift the pool data in one chunk
  if (bytesToMove > 0) {
    std::memmove(moveDst, moveSrc, bytesToMove);
  }

  // Update pool write offset by the bytes we freed
  *poolWriteOffset -= shift;

  // Update each SoundSource in this pool to point at the new buffer location
  if (shift > 0 && moveSrc != nullptr) {
    for (uint32_t j = 0; j < count_; ++j) {
      if (j == index) {
        continue;
      }
      auto *buf = static_cast<uint8_t *>(wav_[j].GetSampleBuffer(0));
      if (buf && buf >= poolBase && buf < poolBase + poolLimit &&
          buf >= moveSrc) {
        wav_[j].SetSampleBuffer(reinterpret_cast<short *>(buf - shift));
      }
    }
  }

  // Shift wav/name entries down in the arrays
  for (uint32_t j = index; j < count_ - 1; ++j) {
    wav_[j] = std::move(wav_[j + 1]);
    memcpy(nameStore_[j], nameStore_[j + 1],
           MAX_INSTRUMENT_FILENAME_LENGTH + 1);
  }

  // clear the last slot (now unused)
  wav_[count_ - 1].Close();
  nameStore_[count_ - 1][0] = '\0';

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
