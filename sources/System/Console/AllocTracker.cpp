/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Simple allocation tracker for debugging heap usage.
 * Logs every C++ allocation and keeps a running outstanding count.
 */

#include "System/Console/Trace.h"

#include <atomic>
#include <cstdlib>
#include <new>

namespace {

std::atomic<size_t> g_outstanding{0};
std::atomic<size_t> g_outstanding_bytes{0};
std::atomic<size_t> g_peak_bytes{0};
thread_local bool g_inLog = false;

struct AllocEntry {
  void *ptr = nullptr;
  size_t size = 0;
};

constexpr size_t kMaxAllocEntries = 128;
AllocEntry g_entries[kMaxAllocEntries];
std::atomic_flag g_entriesLock = ATOMIC_FLAG_INIT;

struct SpinLock {
  SpinLock(std::atomic_flag &f) : flag(f) {
    while (flag.test_and_set(std::memory_order_acquire)) {
    }
  }
  ~SpinLock() { flag.clear(std::memory_order_release); }
  std::atomic_flag &flag;
};

inline void log_alloc(const char *op, size_t size, size_t current,
                      size_t current_bytes, size_t peak_bytes) {
  if (g_inLog) {
    return;
  }
  g_inLog = true;
  Trace::Log("ALLOC", "%s size=%zu outstanding=%zu bytes=%zu peak=%zu", op,
             size, current, current_bytes, peak_bytes);
  g_inLog = false;
}

inline void *alloc_impl(size_t size, bool nothrow) {
  void *p = std::malloc(size);
  if (!p) {
    return nullptr;
  }
  {
    SpinLock lock(g_entriesLock);
    for (size_t i = 0; i < kMaxAllocEntries; ++i) {
      if (g_entries[i].ptr == nullptr) {
        g_entries[i].ptr = p;
        g_entries[i].size = size;
        break;
      }
    }
  }
  size_t curr = g_outstanding.fetch_add(1, std::memory_order_relaxed) + 1;
  size_t bytes = g_outstanding_bytes.fetch_add(size, std::memory_order_relaxed) +
                 size;
  size_t peak = g_peak_bytes.load(std::memory_order_relaxed);
  while (bytes > peak &&
         !g_peak_bytes.compare_exchange_weak(peak, bytes,
                                             std::memory_order_relaxed)) {
  }
  log_alloc("new", size, curr, bytes, g_peak_bytes.load(std::memory_order_relaxed));
  return p;
}

inline void dealloc_impl(void *p, size_t size) noexcept {
  if (p) {
    size_t recordedSize = size;
    {
      SpinLock lock(g_entriesLock);
      for (size_t i = 0; i < kMaxAllocEntries; ++i) {
        if (g_entries[i].ptr == p) {
          recordedSize = g_entries[i].size;
          g_entries[i].ptr = nullptr;
          g_entries[i].size = 0;
          break;
        }
      }
    }
    if (recordedSize == 0) {
      recordedSize = size;
    }
    size_t prev = g_outstanding.load(std::memory_order_relaxed);
    if (prev > 0) {
      size_t curr =
          g_outstanding.fetch_sub(1, std::memory_order_relaxed) - 1;
      size_t bytes =
          g_outstanding_bytes.fetch_sub(recordedSize, std::memory_order_relaxed) -
          recordedSize;
      log_alloc("delete", recordedSize, curr, bytes,
                g_peak_bytes.load(std::memory_order_relaxed));
    } else {
      log_alloc("delete", recordedSize, prev,
                g_outstanding_bytes.load(std::memory_order_relaxed),
                g_peak_bytes.load(std::memory_order_relaxed));
    }
    std::free(p);
  }
}

} // namespace

void *operator new(std::size_t size) { return alloc_impl(size, false); }

void *operator new[](std::size_t size) { return alloc_impl(size, false); }

void *operator new(std::size_t size,
                   const std::nothrow_t &) noexcept {
  return alloc_impl(size, true);
}

void *operator new[](std::size_t size,
                     const std::nothrow_t &) noexcept {
  return alloc_impl(size, true);
}

void operator delete(void *p) noexcept { dealloc_impl(p, 0); }

void operator delete[](void *p) noexcept { dealloc_impl(p, 0); }

void operator delete(void *p, std::size_t size) noexcept {
  dealloc_impl(p, size);
}

void operator delete[](void *p, std::size_t size) noexcept {
  dealloc_impl(p, size);
}
