/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "WavFileWriter.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/I_File.h"
#include "System/System/System.h"
#include "WavHeader.h"
#include <algorithm>
#include <cstring>
#include <limits>

WavFileWriter::WavFileWriter(const char *path)
    : sampleCount_(0), file_(0), path_(path ? path : ""), queue_(nullptr),
      freeQueue_(nullptr), taskHandle_(nullptr), writerActive_(false) {
  file_ = FileSystem::GetInstance()->Open(path, "wb");
  if (file_) {
    // Use WavHeaderWriter to write the header
    if (!WavHeaderWriter::WriteHeader(file_, 44100, 2, 16)) {
      Trace::Log("WAVWRITER", "Failed to write WAV header");
      file_->Close();
      SAFE_DELETE(file_);
    }
  }

  if (file_) {
    queue_ = xQueueCreate(kRenderQueueDepth, sizeof(RenderChunk *));
    freeQueue_ = xQueueCreate(kRenderQueueDepth, sizeof(RenderChunk *));
  }

  if (!queue_ || !freeQueue_) {
    Trace::Error("WAVWRITER", "Failed to create render queues");
    if (queue_) {
      vQueueDelete(queue_);
      queue_ = nullptr;
    }
    if (freeQueue_) {
      vQueueDelete(freeQueue_);
      freeQueue_ = nullptr;
    }
  } else if (file_) {
    for (size_t i = 0; i < kRenderQueueDepth; ++i) {
      RenderChunk *chunk = &chunkPool_[i];
      chunk->samples = 0;
      chunk->terminate = false;
      xQueueSend(freeQueue_, &chunk, 0);
    }
    writerActive_ = true;
    BaseType_t taskResult = xTaskCreate(WavFileWriter::WriterTaskThunk, "wavwr",
                                        kWriterTaskStackSize, this,
                                        kWriterTaskPriority, &taskHandle_);
    if (taskResult != pdPASS) {
      Trace::Error("WAVWRITER", "Failed to start writer task");
      writerActive_ = false;
      vQueueDelete(queue_);
      queue_ = nullptr;
    }
  }
};

WavFileWriter::~WavFileWriter() { Close(); }

void WavFileWriter::AddBuffer(fixed *bufferIn, int size) {

  if (!file_)
    return;

  if (!queue_ || !freeQueue_) {
    return;
  }

  RenderChunk *chunk = nullptr;
  if (xQueueReceive(freeQueue_, &chunk, 0) != pdTRUE || chunk == nullptr) {
    Trace::Error("WAVWRITER", "No free render chunks, dropping audio");
    return;
  }

  chunk->samples = static_cast<uint16_t>(size);
  chunk->terminate = false;
  short *s = chunk->data;
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

  if (xQueueSend(queue_, &chunk, 0) != pdTRUE) {
    Trace::Error("WAVWRITER", "Render queue full, dropping audio chunk");
    xQueueSend(freeQueue_, &chunk, 0);
  }
};

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

  if (!WavHeaderWriter::UpdateFileSize(file, numChannels, bytesPerSample)) {
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

  if (queue_) {
    RenderChunk *sentinel = nullptr;
    while (xQueueSend(queue_, &sentinel, portMAX_DELAY) != pdTRUE) {
      vTaskDelay(pdMS_TO_TICKS(1));
    }

    while (writerActive_) {
      vTaskDelay(pdMS_TO_TICKS(1));
    }

    vQueueDelete(queue_);
    queue_ = nullptr;
  }
  if (freeQueue_) {
    vQueueDelete(freeQueue_);
    freeQueue_ = nullptr;
  }

  long finalSize = file_->Tell();
  Trace::Log("WAVWRITER", "Closing render file (%p) samples=%d tell=%ld",
             static_cast<void *>(file_), sampleCount_, finalSize);

  file_->Sync();
  file_->Close();
  SAFE_DELETE(file_);

  FileSystem *fs = FileSystem::GetInstance();
  if (fs && !path_.empty()) {
    I_File *patchFile = fs->Open(path_.c_str(), "r+");
    if (!patchFile) {
      Trace::Error("WAVWRITER", "Failed to reopen %s for header patch",
                   path_.c_str());
    } else {
      patchFile->Seek(0, SEEK_END);
      if (!WavHeaderWriter::UpdateFileSize(patchFile, sampleCount_)) {
        Trace::Log("WAVWRITER", "Failed to update WAV header");
      }
      patchFile->Close();
      delete patchFile;
    }
  }
};

void WavFileWriter::WriterTaskThunk(void *param) {
  auto *self = static_cast<WavFileWriter *>(param);
  if (self) {
    self->WriterTask();
  }
}

void WavFileWriter::WriterTask() {
  if (!queue_ || !file_) {
    writerActive_ = false;
    vTaskDelete(nullptr);
    return;
  }

  RenderChunk *chunk = nullptr;
  while (xQueueReceive(queue_, &chunk, portMAX_DELAY) == pdTRUE) {
    if (chunk == nullptr) {
      break;
    }
    file_->Write(chunk->data, sizeof(short), chunk->samples * 2);
    sampleCount_ += chunk->samples;
    if (freeQueue_) {
      while (xQueueSend(freeQueue_, &chunk, portMAX_DELAY) != pdTRUE) {
        vTaskDelay(pdMS_TO_TICKS(1));
      }
    }
  }

  writerActive_ = false;
  vTaskDelete(nullptr);
}
