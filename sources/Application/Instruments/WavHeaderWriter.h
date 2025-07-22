/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _WAV_HEADER_WRITER_H_
#define _WAV_HEADER_WRITER_H_

#include "System/FileSystem/FileSystem.h"

class WavHeaderWriter {
public:
  // Write WAV header to I_File
  static bool WriteHeader(I_File *file, uint32_t sampleRate = 44100,
                          uint16_t channels = 2, uint16_t bitsPerSample = 16);

  // Update file size in WAV header for I_File
  static bool UpdateFileSize(I_File *file, uint32_t sampleCount);
};

#endif
