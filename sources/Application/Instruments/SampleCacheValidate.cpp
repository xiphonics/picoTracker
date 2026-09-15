/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "SampleCacheValidate.h"
#include <cstring>

bool flashRangeFits(uint32_t offset, uint32_t size, uint32_t base,
                    uint32_t limit) {
  if (offset < base || offset >= limit) {
    return false;
  }
  // Subtraction, not offset + size: a corrupt size must not wrap and pass.
  return size <= limit - offset;
}

SampleCachePairing pairCardSampleWithCache(const char *name, uint32_t diskSize,
                                           const SampleCacheEntry *entries,
                                           size_t entryCount) {
  for (size_t i = 0; i < entryCount; ++i) {
    if (strcmp(entries[i].name, name) == 0) {
      // Same name but a different byte size means the flash copy came from
      // another file. The size is the fingerprint; mtime would be stricter but
      // the filesystem interface has no stat for it.
      return entries[i].sourceDiskSize == diskSize
                 ? SampleCachePairing::Matched
                 : SampleCachePairing::SizeDiffers;
    }
  }
  return SampleCachePairing::NoMatch;
}

bool cacheAndCardAgree(size_t cardSamples, size_t cardSamplesUnchanged,
                       size_t cachedSamples) {
  // Fewer or more samples than the cache means an add or a delete, which shifts
  // pool indices; fewer unchanged than samples means a replace or a rename.
  return cardSamples == cachedSamples && cardSamplesUnchanged == cachedSamples;
}
