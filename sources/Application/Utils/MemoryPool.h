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

  // Scoped accessor for the file index list with key-based recursive locking.
  //
  // Pass 'this' as the key so the same object can re-enter without deadlocking:
  //   auto list = MemoryPool::getFileIndexList(this);
  //
  // A different key (or the first caller) acquires the mutex; the same key
  // simply increments a depth counter and skips re-locking.  The mutex is
  // released only when the depth reaches zero (outermost scope exits).
  class KeyedScopedRef {
  public:
    using VecType = etl::vector<int, MAX_FILE_INDEX_SIZE>;

    KeyedScopedRef(VecType &resource, SysMutex &mutex)
        : resource_(resource), mutex_(mutex) {}

    ~KeyedScopedRef() {
      if (--fileIndexLockDepth_ == 0) {
        fileIndexCurrentKey_ = nullptr;
        mutex_.Unlock();
      }
    }

    VecType *operator->() { return &resource_; }
    VecType &operator*() { return resource_; }
    KeyedScopedRef(const KeyedScopedRef &) = delete;
    KeyedScopedRef &operator=(const KeyedScopedRef &) = delete;

  private:
    VecType &resource_;
    SysMutex &mutex_;
  };

  [[nodiscard]] static KeyedScopedRef getFileIndexList(const void *key) {
    if (fileIndexCurrentKey_ != key) {
      fileIndexListMutex_->Lock();
      fileIndexCurrentKey_ = key;
    }
    ++fileIndexLockDepth_;
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
  static const void *fileIndexCurrentKey_;
  static int fileIndexLockDepth_;
};