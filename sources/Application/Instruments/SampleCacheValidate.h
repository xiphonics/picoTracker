/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _SAMPLE_CACHE_VALIDATE_H_
#define _SAMPLE_CACHE_VALIDATE_H_

#include "Application/Instruments/SampleCacheEntry.h"
#include <cstddef>
#include <cstdint>

// The parts of sample cache validation that are pure decisions. Kept free of
// the filesystem, the flash allocator and the pool - and free of any buffer of
// their own - so they can be tested on the host. See
// tests/samplecache_tests.cpp.

// True when [offset, offset + size) lies entirely within [base, limit) and
// offset is not below base. Uses subtraction rather than offset + size <= limit
// so a corrupt size cannot wrap around and be accepted.
bool flashRangeFits(uint32_t offset, uint32_t size, uint32_t base,
                    uint32_t limit);

enum class SampleCachePairing {
  NoMatch,     // no cached entry has this name
  SizeDiffers, // cached flash came from a different file
  Matched,     // cached flash still describes this exact file
};

// Pair one WAV found on the card with the cache entry that claims to describe
// it. names are compared exactly: a cache hit must reproduce the pool a fresh
// SD load would build, including its sort order.
SampleCachePairing pairCardSampleWithCache(const char *name, uint32_t diskSize,
                                           const SampleCacheEntry *entries,
                                           size_t entryCount);

// True when the card and the cache agree completely: the same number of
// samples, and every one of them still byte-identical to what was cached. A
// difference means a sample was replaced, resized, added or removed, so the
// pool - and every instrument sample index - would differ from a fresh load.
bool cacheAndCardAgree(size_t cardSamples, size_t cardSamplesUnchanged,
                       size_t cachedSamples);

#endif // _SAMPLE_CACHE_VALIDATE_H_
