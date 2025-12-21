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

#include "Externals/etl/include/etl/expected.h"
#include "SoundSource.h"
#include "System/FileSystem/FileSystem.h"
#include "System/System/System.h"
#include "WavFileErrors.h"

#define BUFFER_SIZE 512

class WavFile : public SoundSource {

public:
  WavFile();
  WavFile(WavFile &&) = default;
  WavFile &operator=(WavFile &&) = default;
  WavFile(const WavFile &) = delete;
  WavFile &operator=(const WavFile &) = delete;
  virtual ~WavFile() = default;

  etl::expected<void, WAVEFILE_ERROR> Open(const char *);
  bool IsOpen() const;
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
  FileHandle file_;    // File
  int readBufferSize_; // Read buffer size
  short *samples_;     // sample buffer size (16 bits)
  int sampleBufferSize_;
  int size_;           // number of samples
  int sampleRate_;     // sample rate
  int channelCount_;   // mono / stereo
  int bytePerSample_;  // original file is in 8/16bit
  int dataPosition_;   // offset in file to get to data
  uint32_t readCount_; // remaining bytes to be read from file

  static int bufferChunkSize_;
  static bool initChunkSize_;
  static unsigned char readBuffer_[BUFFER_SIZE];
};
#endif
