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
#include <FreeRTOS.h>
#include <cstddef>
#include <cstdint>
#include <queue.h>
#include <string>
#include <task.h>

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
  static constexpr size_t kRenderQueueDepth = 4;
  static constexpr uint32_t kWriterTaskStackSize = 1024;
  static constexpr UBaseType_t kWriterTaskPriority = 2;

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
  struct RenderChunk {
    uint16_t samples;
    bool terminate;
    short data[MAX_SAMPLE_COUNT * 2];
  };

  static void WriterTaskThunk(void *param);
  void WriterTask();

  int sampleCount_;
  I_File *file_;
  std::string path_;
  QueueHandle_t queue_;
  QueueHandle_t freeQueue_;
  TaskHandle_t taskHandle_;
  bool writerActive_;
  RenderChunk chunkPool_[kRenderQueueDepth];
};
#endif
