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

typedef void (*SampleEditProgressCallback)(uint8_t percent);

class WavFileWriter {
public:
  static constexpr size_t SharedBufferSize = 8 * 1024;

  WavFileWriter();
  WavFileWriter(const char *path);
  ~WavFileWriter();
  bool Open(const char *path);
  bool IsOpen() const;
  // Borrow only while the WAV render writer is idle.
  static void *GetSharedBuffer() { return sharedBuffer_; }
  void AddBuffer(fixed *, int size); // size in samples
  void Close();
  static bool TrimFile(const char *path, uint32_t startFrame, uint32_t endFrame,
                       void *scratchBuffer, uint32_t scratchBufferSize,
                       WavTrimResult &result,
                       SampleEditProgressCallback progressCallback = nullptr);
  static bool
  NormalizeFile(const char *path, void *scratchBuffer,
                uint32_t scratchBufferSize, WavNormalizeResult &result,
                SampleEditProgressCallback progressCallback = nullptr);

private:
  int sampleCount_;
  // Aligned AXI RAM for rendered WAV output, project Save As copies, and WAV
  // import chunks.
  __attribute__((aligned(32))) static uint8_t sharedBuffer_[SharedBufferSize];
  FileHandle file_;
};
#endif
