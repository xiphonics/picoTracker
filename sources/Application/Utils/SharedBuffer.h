/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _SHARED_BUFFER_H_
#define _SHARED_BUFFER_H_

#include <cstddef>

// Shared 8 KiB aligned storage for WAV render output, project Save As copies,
// and WAV import chunks. Copy/import callers borrow it while render output is
// idle.
class SharedBuffer {
public:
  static constexpr std::size_t Size = 8 * 1024;

  static void *Get();
};

#endif // _SHARED_BUFFER_H_
