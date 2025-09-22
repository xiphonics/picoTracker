/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "picoTrackerSamplePool.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/multicore.h"

#define MB 1024 * 1024

#define VERBOSE_FLASH_DEBUG 0

// Maximum sample storage per project (8MB limit for now)
#define SAMPLE_STORAGE_START_MB 8

// Define where sample storage begins in flash
// Use all flash available after binary for samples
extern char __flash_binary_end;
#define FLASH_TARGET_OFFSET                                                    \
  ((((uintptr_t) & __flash_binary_end - 0x10000000u) / FLASH_SECTOR_SIZE) +    \
   1) *                                                                        \
      FLASH_SECTOR_SIZE

// Total flash size depends on hardware:
// - Raspberry Pi Pico: 2MB
// - picoTracker custom hardware: up to 16MB
// We'll detect actual size at runtime if needed

uint32_t picoTrackerSamplePool::flashEraseOffset_ = FLASH_TARGET_OFFSET;
uint32_t picoTrackerSamplePool::flashWriteOffset_ = FLASH_TARGET_OFFSET;
// Initial default value - will be properly set in the constructor based on
// actual flash size
uint32_t picoTrackerSamplePool::flashLimit_ = 0;
// From the SDK, values are not defined in the header file
#define FLASH_RUID_DUMMY_BYTES 4
#define FLASH_RUID_DATA_BYTES 8
#define FLASH_RUID_TOTAL_BYTES                                                 \
  (1 + FLASH_RUID_DUMMY_BYTES + FLASH_RUID_DATA_BYTES)

uint storage_get_flash_capacity() {
  uint8_t txbuf[FLASH_RUID_TOTAL_BYTES] = {0x9f};
  uint8_t rxbuf[FLASH_RUID_TOTAL_BYTES] = {0};
  flash_do_cmd(txbuf, rxbuf, FLASH_RUID_TOTAL_BYTES);

  return 1 << rxbuf[3];
}

picoTrackerSamplePool::picoTrackerSamplePool() : SamplePool() {
  // Detect the actual flash size at runtime
  uint32_t totalFlashSize = storage_get_flash_capacity();

  // Calculate the maximum usable flash for samples
  // This is either the 8MB limit or the actual available flash, whichever is
  // smaller
  uint32_t maxUsableFlash = SAMPLE_STORAGE_START_MB * MB;

  flashLimit_ = totalFlashSize;

  // Set the flash offset to maximum usable flash back from the top of the flash
  // or immediately after the firmware
  flashWriteOffset_ = flashEraseOffset_ =
      flashLimit_ < SAMPLE_STORAGE_START_MB * MB ? FLASH_TARGET_OFFSET
                                                 : SAMPLE_STORAGE_START_MB * MB;

  Trace::Debug("Total flash size: %u bytes", totalFlashSize);
  Trace::Debug("Flash target offset: %u bytes", FLASH_TARGET_OFFSET);
  Trace::Debug("Max usable flash: %u bytes", maxUsableFlash);
  Trace::Debug("Flash limit set to: %u bytes", flashLimit_);
  Trace::Debug("Flash write offset set to: %u bytes", flashWriteOffset_);
}

void picoTrackerSamplePool::Reset() {
  count_ = 0;
  for (int i = 0; i < MAX_SAMPLES; i++) {
    SAFE_DELETE(wav_[i]);
    SAFE_FREE(names_[i]);
  };

  // Reset flash erase and write pointers when we close project
  flashEraseOffset_ = FLASH_TARGET_OFFSET;
  flashWriteOffset_ = FLASH_TARGET_OFFSET;
};

bool picoTrackerSamplePool::loadSample(const char *name) {
  Trace::Log("SAMPLEPOOL", "Loading sample into flash: %s", name);
  // Pause core1 in order to be able to write to flash and ensure core1 is
  // not reading from it, it also disables IRQs on it
  // https://www.raspberrypi.com/documentation/pico-sdk/high_level.html#multicore_lockout
  if (multicore_lockout_victim_is_initialized(1)) {
    multicore_lockout_start_blocking();
  }

  if (count_ == MAX_SAMPLES)
    return false;

  WavFile *wave = WavFile::Open(name);
  if (wave) {
    wav_[count_] = wave;
    names_[count_] = (char *)SYS_MALLOC(strlen(name) + 1);
    strcpy(names_[count_], name);
    count_++;

    if (!LoadInFlash(wave)) {
      Trace::Error("Failed to load sample into flash: %s", name);
      count_--;
      SYS_FREE(names_[count_]);
      names_[count_] = nullptr;
      delete wav_[count_];
      wav_[count_] = nullptr;
      if (multicore_lockout_victim_is_initialized(1)) {
        multicore_lockout_end_blocking();
      }
      return false;
    }

    wave->Close();

    if (multicore_lockout_victim_is_initialized(1)) {
      multicore_lockout_end_blocking();
    }
    return true;
  } else {
    Trace::Error("Failed to load sample:%s", name);
    if (multicore_lockout_victim_is_initialized(1)) {
      multicore_lockout_end_blocking();
    }
    return false;
  }
};

bool picoTrackerSamplePool::LoadInFlash(WavFile *wave) {

  uint32_t FlashBaseBufferSize = wave->GetDiskSize(-1);
  Trace::Debug("File size: %i", FlashBaseBufferSize);

  // Size actually occupied in flash
  uint32_t FlashPageBufferSize =
      ((FlashBaseBufferSize / FLASH_PAGE_SIZE) +
       ((FlashBaseBufferSize % FLASH_PAGE_SIZE) != 0)) *
      FLASH_PAGE_SIZE;
  Trace::Debug("Size in flash: %i (%i 256 byte pages)", FlashPageBufferSize,
               FlashPageBufferSize / FLASH_PAGE_SIZE);

  if (flashWriteOffset_ + FlashPageBufferSize > flashLimit_) {
    Trace::Error(
        "Sample doesn't fit in available Flash (need: %i - avail: %i) ",
        FlashPageBufferSize, flashLimit_ - flashWriteOffset_);
    return false;
  }

  // Set wave base
  wave->SetSampleBuffer((short *)(XIP_BASE + flashWriteOffset_));

  // Any operation on the flash need to ensure that nothing else reads or writes
  // on it We disable IRQs and ensure that we don't have multiprocessing on
  // at this time
  uint32_t irqs = save_and_disable_interrupts();

  // If data doesn't fit in previously erased page, we'll have to erase
  // additional ones
  if (FlashPageBufferSize > (flashEraseOffset_ - flashWriteOffset_)) {
    uint32_t additionalData =
        FlashPageBufferSize - flashEraseOffset_ + flashWriteOffset_;
    uint32_t sectorsToErase = ((additionalData / FLASH_SECTOR_SIZE) +
                               ((additionalData % FLASH_SECTOR_SIZE) != 0)) *
                              FLASH_SECTOR_SIZE;
    Trace::Debug("About to erase %i sectors in flash region 0x%X - 0x%X",
                 sectorsToErase, flashEraseOffset_,
                 flashEraseOffset_ + sectorsToErase);
    // Erase required number of sectors
    flash_range_erase(flashEraseOffset_, sectorsToErase);
    // Move erase pointer to new position
    flashEraseOffset_ += sectorsToErase;
    Trace::Debug("new erase offset: %p", flashEraseOffset_);
  }

  uint32_t offset = 0;
  uint32_t br = 0;
  uint8_t readBuffer[BUFFER_SIZE];

  wave->Rewind();
  wave->Read(&readBuffer, BUFFER_SIZE, &br);
  while (br > 0) {
#if VERBOSE_FLASH_DEBUG
    Trace::Debug("Read %i bytes", br);
#endif
    // We need to write double the bytes if we needed to expand to 16 bit
    // Write size will be either 256 (which is the flash page size) or 512
    uint32_t writeSize = br;
    // Adjust to page size
    writeSize =
        ((writeSize / FLASH_PAGE_SIZE) + ((writeSize % FLASH_PAGE_SIZE) != 0)) *
        FLASH_PAGE_SIZE;

    // There will be trash at the end, but sampleBufferSize_ gives me the
    // bounds
#if VERBOSE_FLASH_DEBUG
    Trace::Debug("About to write %i sectors in flash region 0x%X - 0x%X",
                 writeSize, flashWriteOffset_ + offset,
                 flashWriteOffset_ + offset + writeSize);
#endif

    flash_range_program(flashWriteOffset_, (uint8_t *)readBuffer, writeSize);
    flashWriteOffset_ += writeSize;
    wave->Read(&readBuffer, BUFFER_SIZE, &br);
  }

  // Lastly we restore the IRQs
  restore_interrupts(irqs);
  return true;
};

bool picoTrackerSamplePool::unloadSample(int index) { return false; };

bool picoTrackerSamplePool::CheckSampleFits(int sampleSize) {
  // Calculate flash storage needed (round up to flash page size)
  uint32_t flashNeeded =
      ((sampleSize / FLASH_PAGE_SIZE) + ((sampleSize % FLASH_PAGE_SIZE) != 0)) *
      FLASH_PAGE_SIZE;

  // Check if there's enough space available
  uint32_t availableFlash = flashLimit_ - flashWriteOffset_;

  return flashNeeded <= availableFlash;
}
