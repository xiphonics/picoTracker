/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _WAV_FILE_H_
#define _WAV_FILE_H_

#include "SoundSource.h"
#include "System/FileSystem/FileSystem.h"
#include "System/System/System.h"
#include "WavFileErrors.h"
#include <expected>

#define BUFFER_SIZE 512

class WavFile : public SoundSource {

protected: // Factory - see Load method
  WavFile(I_File *file);

public:
  virtual ~WavFile();
  static std::expected<WavFile *, WAVEFILE_ERROR> Open(const char *);
  virtual void *GetSampleBuffer(int note);
  void SetSampleBuffer(short *ptr);
  virtual int GetSize(int note);
  virtual int GetSampleRate(int note);
  virtual int GetChannelCount(int note);
  virtual int GetRootNote(int note);
  bool GetBuffer(long start, long sampleCount); // values in samples
  virtual float GetLengthInSec();

  uint32_t GetDiskSize(int note);
  bool Rewind();
  bool Read(void *buff, uint32_t btr, uint32_t *bytesRead);

  void Close();
  virtual bool IsMulti() { return false; };

protected:
  long readBlock(long position, long count);

private:
  I_File *file_;           // File
  int32_t readBufferSize_; // Read buffer size
  int16_t *samples_;       // sample buffer size (16 bits)
  int32_t sampleBufferSize_;
  int32_t size_;          // number of samples
  int32_t sampleRate_;    // sample rate
  int32_t channelCount_;  // mono / stereo
  int32_t bytePerSample_; // original file depth (8/16/24/32bit or float)
  uint16_t audioFormat_;  // PCM or IEEE float
  int32_t dataPosition_;  // offset in file to get to data
  uint32_t readCount_;    // remaining bytes to be read from file

  static unsigned char readBuffer_[BUFFER_SIZE];
  static int16_t convertedBuffer_[BUFFER_SIZE / 2];
};
#endif
