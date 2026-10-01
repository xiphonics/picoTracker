/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "SharedBuffer.h"

#include <cstdint>

namespace {
alignas(32) std::uint8_t sharedBuffer[SharedBuffer::Size];
}

void *SharedBuffer::Get() { return sharedBuffer; }
