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

  wav->size_ = header->dataChunkSize /
               (header->numChannels * header->bytesPerSample);
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

  // expand 8 bit data if needed

  unsigned char *src = (unsigned char *)samples_;
  short *dst = samples_;
  for (int i = size - 1; i >= 0; i--) {
    if (bytePerSample_ == 1) {
      dst[i] = (src[i] - 128) * 256;
    } else {
      dst++;
      if (channelCount_ > 1) {
        dst++;
      }
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
  // Calculate read size based on byte per sample
  uint32_t readSize = (bytePerSample_ == 1) ? btr / 2 : btr;

  // Adjust for bytes remaining
  readSize = readCount_ > readSize ? readSize : readCount_;

  // Read from file
  *bytesRead = file_->Read(buff, readSize);

  // For 8-bit samples, we'll expand to 16-bit
  if (bytePerSample_ == 1) {
    // Process 8-bit samples, converting them to 16-bit
    uint8_t *src = (uint8_t *)buff;
    uint16_t *dst = (uint16_t *)buff;

    // We need to process from end to beginning to avoid overwriting data
    for (int i = *bytesRead - 1; i >= 0; i--) {
      dst[i] = (src[i] - 128) * 256;
    }

    // Adjust bytes read count for 16-bit output
    *bytesRead = *bytesRead * 2;
  } else {
    // Process 16-bit samples
    uint16_t *data = (uint16_t *)buff;
    uint32_t sampleCount = *bytesRead / 2; // 16-bit = 2 bytes per sample
  }

  // Update remaining bytes counter
  readCount_ -= readSize;
  return true;
}

void WavFile::Close() {
  file_->Close();
  SAFE_DELETE(file_);
  readBufferSize_ = 0;
};

int WavFile::GetRootNote(int note) { return 60; }
