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
#include "System/FileSystem/FileSystem.h"

class WavFileWriter {
public:
  WavFileWriter(const char *path);
  ~WavFileWriter();
  void AddBuffer(fixed *, int size); // size in samples
  void Close();

private:
  int sampleCount_;
  short *buffer_;
  int bufferSize_;
  I_File *file_;
};
#endif
