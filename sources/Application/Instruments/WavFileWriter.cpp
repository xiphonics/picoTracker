/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "WavFileWriter.h"
#include "WavHeaderWriter.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/I_File.h"
#include "System/System/System.h"

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
