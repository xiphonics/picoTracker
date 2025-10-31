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
    : sampleCount_(0), buffer_(0), bufferSize_(0), file_(0) {
  file_ = FileSystem::GetInstance()->Open(path, "wb");
  if (file_) {
    // Use WavHeaderWriter to write the header
    if (!WavHeaderWriter::WriteHeader(file_, 44100, 2, 16)) {
      Trace::Log("WAVWRITER", "Failed to write WAV header");
      file_->Close();
      SAFE_DELETE(file_);
    }
  }
};

WavFileWriter::~WavFileWriter() { Close(); }

void WavFileWriter::AddBuffer(fixed *bufferIn, int size) {

  if (!file_)
    return;

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
  file_->Write(buffer_, 2, size * 2);
  sampleCount_ += size;
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
  result = {0, 0, 0, 1.0f, false};

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
  result.targetPeak = (bitsPerSample == 8) ? 127 : 32767;

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

  uint32_t targetPeak = static_cast<uint32_t>(result.targetPeak);
  uint32_t gainQ16 =
      static_cast<uint32_t>((static_cast<uint64_t>(targetPeak) << 16) / peak);
  if (gainQ16 == (1u << 16)) {
    Trace::Log("WavFileWriter",
               "Normalize skipped for %s because peak already at target", path);
    file->Close();
    return true;
  }

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

  // Use WavHeaderWriter to update file size
  if (!WavHeaderWriter::UpdateFileSize(file_, sampleCount_)) {
    Trace::Log("WAVWRITER", "Failed to update WAV header");
  }

  file_->Close();
  SAFE_DELETE(file_);
  SAFE_FREE(buffer_);
};
