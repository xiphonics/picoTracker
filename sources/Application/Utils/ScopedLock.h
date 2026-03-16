/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#pragma once

#include "System/Process/SysMutex.h"

// minimal RAII lock guard using the SysMutex interface ************************

class ScopedLock {
public:
  explicit ScopedLock(SysMutex &m) : m_(m) { m_.Lock(); }
  ~ScopedLock() { m_.Unlock(); }
  ScopedLock(const ScopedLock &) = delete;
  ScopedLock &operator=(const ScopedLock &) = delete;

private:
  SysMutex &m_;
};