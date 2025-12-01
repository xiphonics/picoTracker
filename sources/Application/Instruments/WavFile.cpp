/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "WavFile.h"
#include "Application/Model/Config.h"
#include "Foundation/Types/Types.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/I_File.h"
#include "WavHeader.h"
#include <algorithm>
#include <cmath>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int WavFile::bufferChunkSize_ = -1;
unsigned char WavFile::readBuffer_[BUFFER_SIZE];

int16_t ClampToInt16(double sample) {
  const double clamped = std::clamp(sample, -1.0, 1.0);
  double scaled = clamped * 32767.0;
  if (scaled > 32767.0) {
    scaled = 32767.0;
  } else if (scaled < -32768.0) {
    scaled = -32768.0;
  }
  return static_cast<int16_t>(scaled);
}

int16_t ConvertSampleToInt16(const uint8_t *samplePtr, uint16_t audioFormat,
                             int bytePerSample) {
  if (audioFormat == 1) { // PCM
    switch (bytePerSample) {
    case 1: {
      // expand 8-bit to 16-bit
      // 8-bit PCM is unsigned while >8-bit is signed
      return static_cast<int16_t>((static_cast<int16_t>(samplePtr[0]) - 128)
                                  << 8);
    }
    case 2: {
      // signed 16-bit
      int16_t value;
      memcpy(&value, samplePtr, sizeof(value));
      return value;
    }
    case 3: {
      // signed 24-bit
      int32_t value = samplePtr[0] | (samplePtr[1] << 8) |
                      (static_cast<int32_t>(samplePtr[2]) << 16);
      value = (value << 8) >> 8; // Sign extend
      return static_cast<int16_t>(value >> 8);
    }
    case 4: {
      // signed 32-bit
      int32_t value;
      memcpy(&value, samplePtr, sizeof(value));
      return static_cast<int16_t>(value >> 16);
    }
    default:
      break;
    }
  } else if (audioFormat == 3) { // IEEE float
    if (bytePerSample == 4) {
      float value;
      memcpy(&value, samplePtr, sizeof(value));
      return ClampToInt16(static_cast<double>(value));
    }
    if (bytePerSample == 8) {
      double value;
      memcpy(&value, samplePtr, sizeof(value));
      return ClampToInt16(value);
    }
  }

  Trace::Error("WAVFILE: Unsupported format (%u) or byte depth (%d)",
               audioFormat, bytePerSample);
  return 0;
}

WavFile::WavFile(I_File *file) {
  samples_ = 0;
  size_ = 0;
  readBufferSize_ = 0;
  sampleBufferSize_ = 0;
  audioFormat_ = 0;
  file_ = file;
};

WavFile::~WavFile() {
  if (file_) {
    file_->Close();
    delete file_;
  }
};

std::expected<WavFile *, WAVEFILE_ERROR> WavFile::Open(const char *name) {
  // Trace::Log("WAVFILE", "wave open from %s", name);

  // open file
  FileSystem *fs = FileSystem::GetInstance();
  I_File *file = fs->Open(name, "r");

  if (!file)
    return std::unexpected(INVALID_FILE);

  auto header = WavHeaderWriter::ReadHeader(file);
  if (!header) {
    file->Close();
    delete file;
    return std::unexpected(header.error());
  }

  WavFile *wav = new WavFile(file);

  wav->sampleRate_ = header->sampleRate;
  wav->channelCount_ = header->numChannels;
  wav->bytePerSample_ = header->bytesPerSample;
  wav->audioFormat_ = header->audioFormat;

  Trace::Debug("File data bytes: %u", header->dataChunkSize);

  wav->size_ =
      header->dataChunkSize / (header->numChannels * header->bytesPerSample);
  Trace::Debug("File sample count: %i", wav->size_);

  // All samples are saved as 16bit/sample in memory
  wav->sampleBufferSize_ = wav->size_ * header->numChannels * 2;
  Trace::Debug("File sampleBufferSize_: %i", wav->sampleBufferSize_);

  wav->readCount_ = header->dataChunkSize;
  wav->dataPosition_ = header->dataOffset;

  file->Seek(header->dataOffset, SEEK_SET);
  return wav;
};

void *WavFile::GetSampleBuffer(int note) { return samples_; };

void WavFile::SetSampleBuffer(short *ptr) { samples_ = ptr; }

int WavFile::GetSize(int note) { return size_; };

int WavFile::GetChannelCount(int note) { return channelCount_; };

int WavFile::GetSampleRate(int note) { return sampleRate_; };

float WavFile::GetLengthInSec() { return (float)size_ / sampleRate_; };

long WavFile::readBlock(long start, long size) {
  if (size > readBufferSize_) {
    readBufferSize_ = size;
  }
  file_->Seek(start, SEEK_SET);
  file_->Read(readBuffer_, size);
  return size;
};

bool WavFile::GetBuffer(long start, long size) {
  samples_ = (short *)readBuffer_;

  // compute the file buffer size we need to read

  int bufferSize = size * channelCount_ * bytePerSample_;
  int bufferStart = dataPosition_ + start * channelCount_ * bytePerSample_;

  // Read the buffer but in small chunk to let the system breathe
  // if the files are big

  int count = bufferSize;
  int offset = 0;
  char *ptr = (char *)samples_;
  int readSize = (bufferChunkSize_ > 0) ? bufferChunkSize_
                 : count > 4096         ? 4096
                                        : count;
  // cap the first iteration
  readSize = std::min(readSize, BUFFER_SIZE);

  while (count > 0) {
    readSize = (count > readSize) ? readSize : count;
    // cap to buffer fize
    readSize = std::min(readSize, BUFFER_SIZE);
    readBlock(bufferStart, readSize);
    memcpy(ptr + offset, readBuffer_, readSize);
    bufferStart += readSize;
    count -= readSize;
    offset += readSize;
  }

  int32_t totalSamples = size * channelCount_;
  auto *src = reinterpret_cast<uint8_t *>(samples_);
  auto *dst = reinterpret_cast<int16_t *>(samples_);
  if (bytePerSample_ == 1) {
    // Expanding 8-bit to 16-bit; convert backward to avoid overwrite
    for (int32_t i = totalSamples - 1; i >= 0; --i) {
      const uint8_t *samplePtr = src + i * bytePerSample_;
      dst[i] = ConvertSampleToInt16(samplePtr, audioFormat_, bytePerSample_);
    }
  } else {
    // retain or shring width for samples >= 16-bits
    for (int32_t i = 0; i < totalSamples; ++i) {
      const uint8_t *samplePtr = src + i * bytePerSample_;
      dst[i] = ConvertSampleToInt16(samplePtr, audioFormat_, bytePerSample_);
    }
  }
  return true;
};

uint32_t WavFile::GetDiskSize(int note) { return sampleBufferSize_; }

// rewind to start of data (no header)
bool WavFile::Rewind() {
  file_->Seek(dataPosition_, SEEK_SET);
  readCount_ = size_ * channelCount_ * bytePerSample_;
  return true;
};

// incrementally read file, use rewind method to go to beginning
bool WavFile::Read(void *buff, uint32_t btr, uint32_t *bytesRead) {
  if (!buff || !bytesRead) {
    return false;
  }

  *bytesRead = 0;

  if (btr == 0 || readCount_ == 0) {
    return true;
  }

  // dst is always 16-bit
  uint32_t dstFrameSize = channelCount_ * 2;
  // src can be 8/16/24-bit
  uint32_t srcFrameSize = channelCount_ * bytePerSample_;
  // the max number of frames we can read (floor)
  uint32_t dstFrames = btr / dstFrameSize;
  // Also cap by how many source bytes fit in caller buffer to avoid overflow
  uint32_t srcFramesByBuffer = (srcFrameSize > 0) ? (btr / srcFrameSize) : 0;
  // the number of frames that still have to be read
  uint32_t srcFrames = readCount_ / srcFrameSize;

  uint32_t framesToRead = std::min({dstFrames, srcFrames, srcFramesByBuffer});
  if (framesToRead == 0) {
    return true;
  }
  uint32_t readSize = framesToRead * srcFrameSize;

  uint32_t actualBytesRead = file_->Read(buff, readSize);
  // We should have enough capacity to read all of readSize, if we don't, we
  // need to adjust the file position to where we actually read, adjusting to a
  // full frame
  uint32_t missing = actualBytesRead % srcFrameSize;
  // if we have reminder bytes to read, rewind to last frame read in preparation
  // for next iteration
  if (missing != 0) {
    file_->Seek(-static_cast<int>(missing), SEEK_CUR);
    actualBytesRead -= missing;
  }

  // If what we actually read was less than a frame we could end up with 0 bytes
  // read. Assume the rest of the file does not contain a full frame (unlikely)
  if (actualBytesRead == 0) {
    return true;
  }

  uint32_t framesRead = actualBytesRead / srcFrameSize;
  uint32_t totalSamples = framesRead * channelCount_;

  // TODO: we repeat this logic in two places
  // Now adjust the samples
  uint8_t *src = static_cast<uint8_t *>(buff);
  int16_t *dst = static_cast<int16_t *>(buff);
  if (bytePerSample_ == 1) {
    // Expanding 8-bit to 16-bit; convert backward to avoid overwrite
    for (int32_t i = static_cast<int32_t>(totalSamples) - 1; i >= 0; --i) {
      const uint8_t *samplePtr = src + i * bytePerSample_;
      dst[i] = ConvertSampleToInt16(samplePtr, audioFormat_, bytePerSample_);
    }
  } else {
    // retain or shrink width
    for (uint32_t i = 0; i < totalSamples; ++i) {
      const uint8_t *samplePtr = src + i * bytePerSample_;
      dst[i] = ConvertSampleToInt16(samplePtr, audioFormat_, bytePerSample_);
    }
  }
  *bytesRead = framesRead * dstFrameSize;

  readCount_ -= actualBytesRead;
  return true;
}

void WavFile::Close() {
  file_->Close();
  SAFE_DELETE(file_);
  readBufferSize_ = 0;
};

int WavFile::GetRootNote(int note) { return 60; }
