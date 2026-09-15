/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _SAMPLE_CACHE_ENTRY_H_
#define _SAMPLE_CACHE_ENTRY_H_

#include "Application/Persistency/PersistenceConstants.h"
#include <cstdint>

// On-disk identity of the per-project sample cache. Kept next to the entry
// type (rather than in PersistencyService.h) so that low level audio classes
// can reference it without dragging in the persistence layer.
#define PROJECT_SAMPLES_CACHE_FILE "/.current.samples"
#define PROJECT_SAMPLES_CACHE_MAGIC 0x50545343u // 'PTSC'
#define PROJECT_SAMPLES_CACHE_VERSION 1

// One pooled sample as recorded in the cache: where its 16 bit PCM lives in
// flash plus the WAV metadata needed to rebuild a WavFile without touching the
// SD card.
struct SampleCacheEntry {
  char name[MAX_INSTRUMENT_FILENAME_LENGTH + 1];
  uint32_t flashOffset;
  uint32_t sampleBufferSize;
  uint32_t size;
  uint32_t sampleRate;
  uint16_t channelCount;
  uint16_t bytePerSample;
  uint16_t audioFormat;
};

#endif // _SAMPLE_CACHE_ENTRY_H_
