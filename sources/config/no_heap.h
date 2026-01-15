/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef NO_HEAP_H
#define NO_HEAP_H

#include <cstddef>
#include <new>

#if defined(__GNUC__) || defined(__clang__)
#define NO_HEAP_ERROR                                                          \
  __attribute__((                                                              \
      error("dynamic allocation is disabled; use static/stack storage")))
#else
#define NO_HEAP_ERROR
#endif

void *operator new(std::size_t) NO_HEAP_ERROR;
void *operator new[](std::size_t) NO_HEAP_ERROR;
void *operator new(std::size_t, const std::nothrow_t &) noexcept NO_HEAP_ERROR;
void *operator new[](std::size_t,
                     const std::nothrow_t &) noexcept NO_HEAP_ERROR;

#undef NO_HEAP_ERROR

#endif
