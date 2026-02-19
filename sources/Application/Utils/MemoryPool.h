/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "System/FileSystem/FileSystem.h"

#ifdef DEBUG
#ifndef PROTECT_SCRATCH
#define PROTECT_SCRATCH 1
#endif
#endif

constexpr size_t MEMORYPOOL_SCRATCH_SIZE = 1024;

class MemoryPool {
private:
  static char buffer_[MEMORYPOOL_SCRATCH_SIZE];
#if PROTECT_SCRATCH
  static bool used_;
#endif

public:
  static etl::vector<int, MAX_FILE_INDEX_SIZE> fileIndexList;

  static void *Acquire();
  static void Release();
};