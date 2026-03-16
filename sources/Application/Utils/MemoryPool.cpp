/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "MemoryPool.h"

#ifdef ADV
#include "Adapters/adv/mutex/advMutex.h"
using Mutex = advMutex;
#else
#include "Adapters/picoTracker/mutex/picoTrackerMutex.h"
using Mutex = picoTrackerMutex;
#endif

// globally available 1k scratch buffer for temporary use, e.g. for file
// loading, string rendering, file sorting
char MemoryPool::buffer_[MEMORYPOOL_SCRATCH_SIZE];
static Mutex s_scratchBufferMutex;
SysMutex *MemoryPool::scratchBufferMutex_ = &s_scratchBufferMutex;

// globally available list of file indexes for the current directory
etl::vector<int, MAX_FILE_INDEX_SIZE> MemoryPool::fileIndexList_;
static Mutex s_fileIndexListMutex;
SysMutex *MemoryPool::fileIndexListMutex_ = &s_fileIndexListMutex;
const void *MemoryPool::fileIndexCurrentKey_ = nullptr;
int MemoryPool::fileIndexLockDepth_ = 0;
