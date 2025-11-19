/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "WavFileWriter.h"
#include "Services/Audio/AudioDriver.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/I_File.h"
#include "System/System/System.h"
#include "WavHeader.h"
#include <algorithm>
#include <cstring>
#include <limits>

namespace {

constexpr uint32_t kRenderSampleRate = 44100;
constexpr uint32_t kRenderChannels = 2;
constexpr uint32_t kRenderBytesPerSample = 2;
constexpr uint32_t kRenderPreallocSeconds = 20; // 20 seconds
constexpr uint32_t kWavHeaderBytes = 44;
constexpr uint64_t kRenderPreallocBytes =
    static_cast<uint64_t>(kRenderSampleRate) * kRenderChannels *
    kRenderBytesPerSample * kRenderPreallocSeconds;
#if defined(ADV)
constexpr size_t kWriterQueueLength = 16;
constexpr uint16_t kWriterTaskStackWords = 2048;
constexpr UBaseType_t kWriterTaskPriority = tskIDLE_PRIORITY + 1;
#endif

} // namespace

WavFileWriter::WavFileWriter(const char *path)
#if defined(ADV)
    : writeQueue_(nullptr), writerTask_(nullptr), writerFinished_(nullptr),
      useAsyncWriter_(false),
#else
    :
#endif
      sampleCount_(0), buffer_(0), bufferSize_(0), file_(0), timingCount_(0),
      bufferIndex_(0) {
  file_ = FileSystem::GetInstance()->Open(path, "wb");
  if (file_) {
    // Preallocate before writing the header (FatFs requires zero-sized file)
    PreAllocateRenderFile();
    file_->Seek(0, SEEK_SET);
    // Use WavHeaderWriter to write the header
    if (!WavHeaderWriter::WriteHeader(file_, 44100, 2, 16)) {
      Trace::Log("WAVWRITER", "Failed to write WAV header");
      file_->Close();
      SAFE_DELETE(file_);
    } else {
      file_->Seek(kWavHeaderBytes, SEEK_SET);
#if defined(ADV)
      useAsyncWriter_ = StartWriter();
#endif
    }
  }
};

WavFileWriter::~WavFileWriter() { Close(); }

void WavFileWriter::AddBuffer(fixed *bufferIn, int size) {

  if (!file_)
    return;

  System *system = System::GetInstance();
  uint32_t startTime = system->Millis();

  // allocate a short buffer for transfer

  if (size > bufferSize_) {
    SAFE_FREE(buffer_);
    buffer_ = (short *)malloc(size * 2 * sizeof(short));
    bufferSize_ = size;
  };

  if (!buffer_)
    return;

  short *s = buffer_;
  fixed *p = bufferIn;

  fixed v;
  fixed f_32767 = i2fp(32767);
  fixed f_m32768 = i2fp(-32768);

  for (int i = 0; i < size * 2; i++) {
    // Left
    v = *p++;
    if (v > f_32767) {
      v = f_32767;
    } else if (v < f_m32768) {
      v = f_m32768;
    }
    *s++ = short(fp2i(v));
  };
#if defined(ADV)
  bool handled = false;
  if (useAsyncWriter_ && writeQueue_) {
    const uint32_t frameCount = static_cast<uint32_t>(size);
    const size_t copyBytes =
        static_cast<size_t>(frameCount) * 2U * sizeof(short);
    short *jobBuffer = static_cast<short *>(pvPortMalloc(copyBytes));
    if (jobBuffer) {
      memcpy(jobBuffer, buffer_, copyBytes);
      RenderWriteJob job{jobBuffer, frameCount};
      if (EnqueueWriteJob(job)) {
        handled = true;
      } else {
        Trace::Error("WAVWRITER", "Render write queue full, writing inline");
        ProcessWriteJob(job);
        handled = true;
      }
    } else {
      Trace::Error("WAVWRITER", "Failed to allocate render queue buffer");
    }
  }
  if (!handled) {
    WriteSamples(buffer_, static_cast<uint32_t>(size));
  }
#else
  WriteSamples(buffer_, static_cast<uint32_t>(size));
#endif

  uint32_t duration = system->Millis() - startTime;
  if (timingCount_ < kMaxTimingEntries) {
    timings_[timingCount_++] = {bufferIndex_, duration};
  }
  bufferIndex_++;
};

bool WavFileWriter::WriteSamples(const short *data, uint32_t frameCount) {
  if (!file_ || frameCount == 0) {
    return false;
  }

  const size_t samplePairs = static_cast<size_t>(frameCount) * 2U;
  int written = file_->Write(data, sizeof(short),
                             static_cast<int>(samplePairs));
  if (written != static_cast<int>(samplePairs)) {
    Trace::Error("WAVWRITER", "Failed writing render data");
    return false;
  }
  sampleCount_ += frameCount;
  return true;
}

bool WavFileWriter::PreAllocateRenderFile() {
  if (!file_) {
    return false;
  }

  const uint64_t totalBytes = kWavHeaderBytes + kRenderPreallocBytes;
  Trace::Log("WAVWRITER", "Attempting to preallocate %llu bytes for render file",
             static_cast<unsigned long long>(totalBytes));
  bool preallocated = file_->PreAllocate(totalBytes);
  if (preallocated) {
    Trace::Log("WAVWRITER", "Preallocated %llu bytes for render file",
               static_cast<unsigned long long>(totalBytes));
  } else {
    Trace::Error("WAVWRITER", "Preallocate failed for %llu-byte render target",
                 static_cast<unsigned long long>(totalBytes));
  }
  return preallocated;
}

#if defined(ADV)
bool WavFileWriter::StartWriter() {
  if (writeQueue_) {
    return true;
  }

  writeQueue_ = xQueueCreate(kWriterQueueLength, sizeof(RenderWriteJob));
  if (!writeQueue_) {
    Trace::Error("WAVWRITER", "Failed to create render queue");
    return false;
  }

  writerFinished_ = xSemaphoreCreateBinary();
  if (!writerFinished_) {
    Trace::Error("WAVWRITER", "Failed to create writer semaphore");
    vQueueDelete(writeQueue_);
    writeQueue_ = nullptr;
    return false;
  }

  BaseType_t result =
      xTaskCreate(WriterTaskEntry, "RenderWriter", kWriterTaskStackWords, this,
                  kWriterTaskPriority, &writerTask_);
  if (result != pdPASS) {
    Trace::Error("WAVWRITER", "Failed to create writer task");
    vSemaphoreDelete(writerFinished_);
    writerFinished_ = nullptr;
    vQueueDelete(writeQueue_);
    writeQueue_ = nullptr;
    return false;
  }
  return true;
}

void WavFileWriter::StopWriter() {
  if (!useAsyncWriter_) {
    return;
  }

  if (writeQueue_) {
    RenderWriteJob job{nullptr, 0};
    xQueueSend(writeQueue_, &job, portMAX_DELAY);
  }
  if (writerFinished_) {
    xSemaphoreTake(writerFinished_, portMAX_DELAY);
    vSemaphoreDelete(writerFinished_);
    writerFinished_ = nullptr;
  }
  if (writeQueue_) {
    vQueueDelete(writeQueue_);
    writeQueue_ = nullptr;
  }
  writerTask_ = nullptr;
  useAsyncWriter_ = false;
}

bool WavFileWriter::EnqueueWriteJob(const RenderWriteJob &job) {
  if (!writeQueue_) {
    return false;
  }
  return xQueueSend(writeQueue_, &job, 0) == pdTRUE;
}

void WavFileWriter::ProcessWriteJob(const RenderWriteJob &job) {
  if (!job.data) {
    return;
  }
  WriteSamples(job.data, job.frameCount);
  vPortFree(job.data);
}

void WavFileWriter::WriterTaskEntry(void *param) {
  WavFileWriter *self = static_cast<WavFileWriter *>(param);
  if (self) {
    self->WriterTaskLoop();
  }
}

void WavFileWriter::WriterTaskLoop() {
  RenderWriteJob job{};
  while (writeQueue_) {
    if (xQueueReceive(writeQueue_, &job, portMAX_DELAY) != pdTRUE) {
      continue;
    }
    if (job.data == nullptr) {
      break;
    }
    ProcessWriteJob(job);
  }
  if (writerFinished_) {
    xSemaphoreGive(writerFinished_);
  }
  vTaskDelete(nullptr);
}
#endif

bool WavFileWriter::TrimFile(const char *path, uint32_t startFrame,
                             uint32_t endFrame, void *scratchBuffer,
                             size_t scratchBufferSize, WavTrimResult &result) {
  result = {0, 0, 0, 0, false};

  if (!path) {
    Trace::Error("WavFileWriter: TrimFile received null path");
    return false;
  }
  if (!scratchBuffer || scratchBufferSize == 0) {
    Trace::Error("WavFileWriter: TrimFile received invalid scratch buffer");
    return false;
  }
  if (endFrame < startFrame) {
    Trace::Error("WavFileWriter: Trim range invalid (%u < %u)", endFrame,
                 startFrame);
    return false;
  }

  auto fs = FileSystem::GetInstance();
  if (!fs) {
    Trace::Error("WavFileWriter: FileSystem unavailable");
    return false;
  }

  I_File *file = fs->Open(path, "r+");
  if (!file) {
    Trace::Error("WavFileWriter: Failed to open %s for trimming", path);
    return false;
  }

  auto headerInfo = WavHeaderWriter::ReadHeader(file);
  if (!headerInfo) {
    Trace::Error("WavFileWriter: Failed to parse WAV header from %s", path);
    file->Close();
    return false;
  }

  const uint16_t numChannels = headerInfo->numChannels;
  const uint16_t bytesPerSample = headerInfo->bytesPerSample;
  const uint16_t bitsPerSample = headerInfo->bitsPerSample;
  const uint32_t dataChunkSize = headerInfo->dataChunkSize;

  if (numChannels == 0 || bitsPerSample == 0) {
    Trace::Error("WavFileWriter: Unsupported WAV format (channels=%u, bits=%u) "
                 "in %s",
                 numChannels, bitsPerSample, path);
    file->Close();
    return false;
  }

  uint32_t bytesPerFrame = numChannels * bytesPerSample;
  if (bytesPerFrame == 0) {
    Trace::Error("WavFileWriter: Invalid bytes per frame for %s", path);
    file->Close();
    return false;
  }

  uint32_t totalFrames = bytesPerFrame ? (dataChunkSize / bytesPerFrame) : 0;
  if (totalFrames == 0) {
    Trace::Error("WavFileWriter: Sample has no audio frames in %s", path);
    file->Close();
    return false;
  }

  uint32_t clampedStart =
      std::min(startFrame, totalFrames > 0 ? totalFrames - 1 : 0);
  uint32_t clampedEnd =
      std::min(endFrame, totalFrames > 0 ? totalFrames - 1 : 0);

  if (clampedStart > clampedEnd) {
    clampedStart = clampedEnd;
  }

  uint32_t framesToKeep =
      (clampedEnd >= clampedStart) ? (clampedEnd - clampedStart + 1) : 0;
  if (framesToKeep == 0) {
    Trace::Error("WavFileWriter: Trim would result in empty sample");
    file->Close();
    return false;
  }

  result.totalFrames = totalFrames;
  result.clampedStart = clampedStart;
  result.clampedEnd = clampedEnd;
  result.framesKept = framesToKeep;

  if (clampedStart == 0 && framesToKeep == totalFrames) {
    file->Close();
    Trace::Log("WavFileWriter",
               "Trim skipped because selection spans entire sample");
    return true;
  }

  const uint32_t headerDataOffset = headerInfo->dataOffset;
  uint32_t readOffset = headerDataOffset + clampedStart * bytesPerFrame;
  uint32_t writeOffset = headerDataOffset;
  uint32_t bytesRemaining = framesToKeep * bytesPerFrame;

  while (bytesRemaining > 0) {
    uint32_t chunkSize = std::min<uint32_t>(
        bytesRemaining, static_cast<uint32_t>(scratchBufferSize));

    file->Seek(readOffset, SEEK_SET);
    int bytesRead = file->Read(scratchBuffer, chunkSize);
    if (bytesRead <= 0) {
      Trace::Error("WavFileWriter: Failed reading sample data during trim");
      file->Close();
      return false;
    }

    file->Seek(writeOffset, SEEK_SET);
    int bytesWritten = file->Write(scratchBuffer, 1, bytesRead);
    if (bytesWritten != bytesRead) {
      Trace::Error("WavFileWriter: Failed writing trimmed data");
      file->Close();
      return false;
    }

    readOffset += bytesRead;
    writeOffset += bytesRead;
    bytesRemaining -= static_cast<uint32_t>(bytesRead);
  }

  const uint32_t newDataSize = framesToKeep * bytesPerFrame;
  file->Seek(headerDataOffset + newDataSize, SEEK_SET);
  if (!WavHeaderWriter::UpdateFileSize(file, framesToKeep, numChannels,
                                       bytesPerSample)) {
    Trace::Error("WavFileWriter: Failed updating WAV header after trim");
    file->Close();
    return false;
  }

  file->Close();
  result.trimmed = true;
  return true;
}

bool WavFileWriter::NormalizeFile(const char *path, void *scratchBuffer,
                                  size_t scratchBufferSize,
                                  WavNormalizeResult &result) {
  result = {
      .totalFrames = 0,
      .peakBefore = 0,
      .targetPeak = 0,
      .gainApplied = 1.0f,
      .normalized = false,
  };

  if (!path) {
    Trace::Error("WavFileWriter: NormalizeFile received null path");
    return false;
  }
  if (!scratchBuffer || scratchBufferSize == 0) {
    Trace::Error("WavFileWriter: NormalizeFile received invalid scratch "
                 "buffer");
    return false;
  }

  auto fs = FileSystem::GetInstance();
  if (!fs) {
    Trace::Error("WavFileWriter: FileSystem unavailable");
    return false;
  }

  I_File *file = fs->Open(path, "r+");
  if (!file) {
    Trace::Error("WavFileWriter: Failed to open %s for normalization", path);
    return false;
  }

  auto headerInfo = WavHeaderWriter::ReadHeader(file);
  if (!headerInfo) {
    Trace::Error("WavFileWriter: Failed to parse WAV header from %s", path);
    file->Close();
    return false;
  }

  const uint16_t numChannels = headerInfo->numChannels;
  const uint16_t bytesPerSample = headerInfo->bytesPerSample;
  const uint16_t bitsPerSample = headerInfo->bitsPerSample;
  const uint32_t dataChunkSize = headerInfo->dataChunkSize;
  const uint32_t dataOffset = headerInfo->dataOffset;

  if (numChannels == 0 || numChannels > 2 || bytesPerSample == 0 ||
      dataChunkSize == 0) {
    Trace::Error("WavFileWriter: Unsupported WAV format for normalization: "
                 "%s",
                 path);
    file->Close();
    return false;
  }

  const uint32_t bytesPerFrame = numChannels * bytesPerSample;
  if (bytesPerFrame == 0) {
    Trace::Error("WavFileWriter: Invalid bytes per frame for %s", path);
    file->Close();
    return false;
  }

  if (scratchBufferSize < bytesPerFrame) {
    Trace::Error("WavFileWriter: Scratch buffer too small (%zu) for frame "
                 "size %u",
                 scratchBufferSize, bytesPerFrame);
    file->Close();
    return false;
  }

  const uint32_t totalFrames =
      bytesPerFrame ? (dataChunkSize / bytesPerFrame) : 0;
  result.totalFrames = totalFrames;
  const uint32_t fullScalePeak = (bitsPerSample == 8) ? 127U : 32767U;
  result.targetPeak = static_cast<int32_t>(fullScalePeak);

  if (totalFrames == 0) {
    Trace::Error("WavFileWriter: Sample has no audio frames in %s", path);
    file->Close();
    return false;
  }

  uint32_t usableChunk = static_cast<uint32_t>(
      (scratchBufferSize / bytesPerFrame) * bytesPerFrame);
  if (usableChunk == 0) {
    usableChunk = bytesPerFrame;
  }

  uint32_t bytesRemaining = dataChunkSize;
  uint32_t readOffset = dataOffset;
  int32_t peak = 0;

  while (bytesRemaining > 0) {
    uint32_t chunkSize = std::min<uint32_t>(bytesRemaining, usableChunk);

    file->Seek(readOffset, SEEK_SET);
    int bytesRead = file->Read(scratchBuffer, chunkSize);
    if (bytesRead != static_cast<int>(chunkSize)) {
      Trace::Error(
          "WavFileWriter: Failed reading sample data during normalization");
      file->Close();
      return false;
    }

    if (bitsPerSample == 8) {
      uint8_t *samples = static_cast<uint8_t *>(scratchBuffer);
      for (int i = 0; i < bytesRead; ++i) {
        int32_t centered = static_cast<int32_t>(samples[i]) - 128;
        int32_t absValue = centered < 0 ? -centered : centered;
        if (absValue > peak) {
          peak = absValue;
        }
      }
    } else {
      int16_t *samples = static_cast<int16_t *>(scratchBuffer);
      int32_t sampleCount = bytesRead / sizeof(int16_t);
      for (int i = 0; i < sampleCount; ++i) {
        int32_t sample = static_cast<int32_t>(samples[i]);
        // we need magnitude so need positive value
        sample = (sample < 0) ? -sample : sample;
        if (sample > peak) {
          peak = sample;
        }
      }
    }

    readOffset += static_cast<uint32_t>(bytesRead);
    bytesRemaining -= static_cast<uint32_t>(bytesRead);
  }

  result.peakBefore = peak;
  if (peak <= 0) {
    Trace::Log("WavFileWriter",
               "Normalize skipped for %s due to zero detected peak", path);
    file->Close();
    return true;
  }

  if (static_cast<uint32_t>(peak) >= fullScalePeak) {
    Trace::Log(
        "WavFileWriter",
        "Normalize skipped for %s because peak already at or above full scale",
        path);
    file->Close();
    return true;
  }
  // `gainQ16` calculates the gain factor required to normalize the audio.
  // It's represented in Q16 fixed-point format (multiplied by 2^16) to maintain
  // precision for fractional gain values. The fullscale peak is cast to
  // `uint64_t` before the left shift to prevent overflow during the
  // multiplication by 2^16.
  uint32_t gainQ16 = static_cast<uint32_t>(
      (static_cast<uint64_t>(fullScalePeak) << 16) / peak);

  bytesRemaining = dataChunkSize;
  readOffset = dataOffset;

  const int32_t minValue = bitsPerSample == 8 ? -128 : -32768;
  const int32_t maxValue = bitsPerSample == 8 ? 127 : 32767;

  while (bytesRemaining > 0) {
    uint32_t chunkSize = std::min<uint32_t>(bytesRemaining, usableChunk);

    file->Seek(readOffset, SEEK_SET);
    int bytesRead = file->Read(scratchBuffer, chunkSize);
    if (bytesRead != static_cast<int>(chunkSize)) {
      Trace::Error(
          "WavFileWriter: Failed reading sample data during normalization");
      file->Close();
      return false;
    }

    if (bitsPerSample == 8) {
      uint8_t *samples = static_cast<uint8_t *>(scratchBuffer);
      for (int i = 0; i < bytesRead; ++i) {
        int32_t centered = static_cast<int32_t>(samples[i]) - 128;
        int64_t scaled = static_cast<int64_t>(centered) * gainQ16;
        if (scaled >= 0) {
          scaled += 0x8000;
        } else {
          scaled -= 0x8000;
        }
        scaled >>= 16;
        if (scaled > maxValue) {
          scaled = maxValue;
        } else if (scaled < minValue) {
          scaled = minValue;
        }
        samples[i] = static_cast<uint8_t>(scaled + 128);
      }
    } else {
      int16_t *samples = static_cast<int16_t *>(scratchBuffer);
      int sampleCount = bytesRead / sizeof(int16_t);
      for (int i = 0; i < sampleCount; ++i) {
        int32_t sample = static_cast<int32_t>(samples[i]);
        int64_t scaled = static_cast<int64_t>(sample) * gainQ16;
        if (scaled >= 0) {
          scaled += 0x8000;
        } else {
          scaled -= 0x8000;
        }
        scaled >>= 16;
        if (scaled > maxValue) {
          scaled = maxValue;
        } else if (scaled < minValue) {
          scaled = minValue;
        }
        samples[i] = static_cast<int16_t>(scaled);
      }
    }

    file->Seek(readOffset, SEEK_SET);
    int bytesWritten = file->Write(scratchBuffer, 1, bytesRead);
    if (bytesWritten != bytesRead) {
      Trace::Error("WavFileWriter: Failed writing normalized data");
      file->Close();
      return false;
    }

    readOffset += static_cast<uint32_t>(bytesRead);
    bytesRemaining -= static_cast<uint32_t>(bytesRead);
  }

  file->Sync();
  file->Close();

  result.normalized = true;
  result.gainApplied =
      static_cast<float>(gainQ16) / static_cast<float>(1u << 16);

  Trace::Log("WavFileWriter",
             "Normalized %s with gain factor %.3f (peak=%d target=%d)", path,
             result.gainApplied, result.peakBefore, result.targetPeak);
  return true;
}

void WavFileWriter::Close() {

  if (!file_)
    return;

#if defined(ADV)
  if (useAsyncWriter_) {
    StopWriter();
  }
#endif

  if (timingCount_ > 0) {
    Trace::Log("RENDER_TRACE", "Render buffer timings:");
    for (size_t i = 0; i < timingCount_; i++) {
      Trace::Log("RENDER_TRACE", "  idx=%lu duration=%lu ms",
                 static_cast<unsigned long>(timings_[i].bufferIndex),
                 static_cast<unsigned long>(timings_[i].durationMs));
    }
    timingCount_ = 0;
    bufferIndex_ = 0;
  }

  // Use WavHeaderWriter to update file size
  if (!WavHeaderWriter::UpdateFileSize(file_, sampleCount_)) {
    Trace::Log("WAVWRITER", "Failed to update WAV header");
  }

  file_->Close();
  SAFE_DELETE(file_);
  SAFE_FREE(buffer_);
};
