/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "MemoryPool.h"

#if PROTECT_SCRATCH
#include "System/Console/Trace.h"
#endif

// globally available 1k scratch buffer for temporary use, e.g. for file loading
// string rendering, file sorting
char MemoryPool::buffer_[MEMORYPOOL_SCRATCH_SIZE];

#if PROTECT_SCRATCH
bool MemoryPool::used_ = false;
#endif
etl::vector<int, MAX_FILE_INDEX_SIZE> MemoryPool::fileIndexList;

void *MemoryPool::Acquire() {
#if PROTECT_SCRATCH
  if (used_) {
    Trace::Error("MemoryPool is already in use!");
  }
  NAssert(used_ == false);
  used_ = true;
#endif
  return buffer_;
}

void MemoryPool::Release() {
#if PROTECT_SCRATCH
  used_ = false;
#endif
}
