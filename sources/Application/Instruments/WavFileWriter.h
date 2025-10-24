/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _WAV_FILE_WRITER_H_
#define _WAV_FILE_WRITER_H_

#include "Application/Utils/fixed.h"
#include "System/FileSystem/FileSystem.h"
#include <cstddef>
#include <cstdint>

struct WavTrimResult {
  uint32_t totalFrames;
  uint32_t clampedStart;
  uint32_t clampedEnd;
  uint32_t framesKept;
  bool trimmed;
};

class WavFileWriter {
public:
  WavFileWriter(const char *path);
  ~WavFileWriter();
  void AddBuffer(fixed *, int size); // size in samples
  void Close();
  static bool TrimFile(const char *path, uint32_t startFrame,
                       uint32_t endFrame, void *scratchBuffer,
                       size_t scratchBufferSize, WavTrimResult &result);

private:
  int sampleCount_;
  short *buffer_;
  int bufferSize_;
  I_File *file_;
};
#endif
