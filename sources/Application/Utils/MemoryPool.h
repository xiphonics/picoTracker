/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#pragma once

#include "System/FileSystem/FileSystem.h"

#include "ScopedLock.h"

constexpr size_t MEMORYPOOL_SCRATCH_SIZE = 1024;

class MemoryPool {
public:
  template <typename T> class ScopedRef {
  public:
    ScopedRef(T &resource, SysMutex &mutex)
        : resource_(resource), lock_(mutex) {}
    T *operator->() { return &resource_; }
    T &operator*() { return resource_; }
    ScopedRef(const ScopedRef &) = delete;
    ScopedRef &operator=(const ScopedRef &) = delete;

  private:
    T &resource_;
    ScopedLock lock_;
  };

  // Scoped accessor for the raw scratch buffer.
  //
  // CORRECT:
  //   auto buf = MemoryPool::getBuffer();
  //   char *p = buf.data();  // lock held until 'buf' goes out of scope
  //
  // WRONG - lock released immediately, p is unprotected:
  //   char *p = MemoryPool::getBuffer().data();
  class ScopedBuffer {
  public:
    ScopedBuffer(char *buf, SysMutex &m) : buf_(buf), lock_(m) {}
    char *data() { return buf_; }
    ScopedBuffer(const ScopedBuffer &) = delete;
    ScopedBuffer &operator=(const ScopedBuffer &) = delete;

  private:
    char *buf_;
    ScopedLock lock_;
  };

  [[nodiscard]] static ScopedRef<etl::vector<int, MAX_FILE_INDEX_SIZE>>
  getFileIndexList() {
    return {fileIndexList_, *fileIndexListMutex_};
  }

  [[nodiscard]] static ScopedBuffer getBuffer() {
    return {buffer_, *scratchBufferMutex_};
  }

private:
  static char buffer_[MEMORYPOOL_SCRATCH_SIZE];
  static etl::vector<int, MAX_FILE_INDEX_SIZE> fileIndexList_;
  static SysMutex *fileIndexListMutex_;
  static SysMutex *scratchBufferMutex_;
};