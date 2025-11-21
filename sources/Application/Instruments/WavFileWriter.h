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
#include "Services/Audio/AudioDriver.h"
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

struct WavNormalizeResult {
  uint32_t totalFrames;
  int32_t peakBefore;
  int32_t targetPeak;
  float gainApplied;
  bool normalized;
};

class WavFileWriter {
public:
  WavFileWriter(const char *path);
  ~WavFileWriter();
  void AddBuffer(fixed *, int size); // size in samples
  void Close();
  static bool TrimFile(const char *path, uint32_t startFrame, uint32_t endFrame,
                       void *scratchBuffer, size_t scratchBufferSize,
                       WavTrimResult &result);
  static bool NormalizeFile(const char *path, void *scratchBuffer,
                            size_t scratchBufferSize,
                            WavNormalizeResult &result);

private:
  int sampleCount_;
  // Buffer in AXI RAM since it has to be reachable by DMA perif
  __attribute__((aligned(32))) static short buffer_[MAX_SAMPLE_COUNT * 2];
  I_File *file_;
};
#endif
