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
#include <stdint.h>
#include <stdlib.h>

int WavFile::bufferChunkSize_ = -1;
unsigned char WavFile::readBuffer_[BUFFER_SIZE];

WavFile::WavFile(I_File *file) {
  samples_ = 0;
  size_ = 0;
  readBufferSize_ = 0;
  sampleBufferSize_ = 0;
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

  while (count > 0) {
    readSize = (count > readSize) ? readSize : count;
    readBlock(bufferStart, readSize);
    memcpy(ptr + offset, readBuffer_, readSize);
    bufferStart += readSize;
    count -= readSize;
    offset += readSize;
  }

  int32_t totalSamples = size * channelCount_;
  if (bytePerSample_ == 1) {
    // expand 8-bit to 16-bit
    unsigned char *src = (unsigned char *)samples_;
    short *dst = samples_;
    for (int32_t i = totalSamples - 1; i >= 0; --i) {
      // 8-bit PCM is unsigned while >8-bit is signed
      dst[i] = (short)((src[i] - 128) << 8);
    }
  } else if (bytePerSample_ == 3) {
    // reduce 24-bit to 16-bit
    unsigned char *src = (unsigned char *)samples_;
    short *dst = samples_;
    for (int32_t i = 0; i < totalSamples; ++i) {
      const uint32_t byteIndex = i * 3;
      // 24-bit PCM sample is 24-bit packed
      int32_t value = src[byteIndex] | (src[byteIndex + 1] << 8) |
                      (src[byteIndex + 2] << 16);
      // Sign extend 24-bit int to 32-bit representation
      value = (value << 8) >> 8;
      // convert back to 16-bit
      dst[i] = (short)(value >> 8);
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
  // the number of frames that still have to be read
  uint32_t srcFrames = readCount_ / srcFrameSize;

  uint32_t framesToRead = (dstFrames < srcFrames) ? dstFrames : srcFrames;
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
  if (bytePerSample_ == 1) {
    // 8 bit samples are unsigned
    uint8_t *src = static_cast<uint8_t *>(buff);
    int16_t *dst = static_cast<int16_t *>(buff);
    for (int32_t i = static_cast<int32_t>(totalSamples) - 1; i >= 0; --i) {
      dst[i] = static_cast<int16_t>((src[i] - 128) << 8);
    }
    *bytesRead = framesRead * dstFrameSize;
  } else if (bytePerSample_ == 2) {
    // nothing to do with 16 bit samples
    *bytesRead = actualBytesRead;
  } else if (bytePerSample_ == 3) {
    // 24 bit samples are 3 byte packed and we have to reconstruct the number
    uint8_t *src = static_cast<uint8_t *>(buff);
    int16_t *dst = static_cast<int16_t *>(buff);
    for (uint32_t i = 0; i < totalSamples; ++i) {
      uint32_t byteIndex = i * 3;
      int32_t value = src[byteIndex] | (src[byteIndex + 1] << 8) |
                      (src[byteIndex + 2] << 16);
      // sign extend
      value = (value << 8) >> 8;
      // save 16-bit sample
      dst[i] = static_cast<int16_t>(value >> 8);
    }
    *bytesRead = framesRead * dstFrameSize;
  } else {
    // This should never happen, is it necessary? should we exit earlier?
    Trace::Error("WAVFILE: Unsupported bytes per sample %d", bytePerSample_);
    return false;
  }

  readCount_ -= actualBytesRead;
  return true;
}

void WavFile::Close() {
  file_->Close();
  SAFE_DELETE(file_);
  readBufferSize_ = 0;
};

int WavFile::GetRootNote(int note) { return 60; }
