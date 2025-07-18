/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "RecordStreamer.h"
#include "System/Console/Trace.h"

RecordStreamer::RecordStreamer() { mode_ = RSM_STOPPED; };

RecordStreamer::~RecordStreamer(){};

bool RecordStreamer::Start(uint16_t *srcBuffer, uint32_t size, bool stereo) {
  Trace::Debug("Starting to stream from record buffer");
  mode_ = RSM_PLAYING;
  srcBuffer_ = srcBuffer;
  bufferSize_ = size;
  stereo_ = stereo;
  srcBufferPos_ = 0;
  return true;
};

void RecordStreamer::Stop() {
  mode_ = RSM_STOPPED;
  Trace::Debug("record streaming stopped");
};

bool RecordStreamer::IsPlaying() { return (mode_ == RSM_PLAYING); }

bool RecordStreamer::Render(fixed *buffer, int samplecount) {

  // See if we're playing
  if (mode_ == RSM_STOPPED) {
    return false;
  }

  // srcBuffer is uint16_t, we advance by one sample per step (L/R/L/R...)
  for (int32_t i = 0; i < samplecount; ++i) {
    // Read one frame from record buffer
    int32_t left = 0, right = 0;

    if (stereo_) {
      // Read L and R
      left = *(int16_t *)&srcBuffer_[srcBufferPos_];
      srcBufferPos_ = (srcBufferPos_ + 1) % bufferSize_;

      right = *(int16_t *)&srcBuffer_[srcBufferPos_];
      srcBufferPos_ = (srcBufferPos_ + 1) % bufferSize_;
    } else {
      // Mono: duplicate for L and R and advance two
      left = *(int16_t *)&srcBuffer_[srcBufferPos_];
      right = left;
      srcBufferPos_ = (srcBufferPos_ + 2) % bufferSize_;
    }

    // Write to output render buffer
    buffer[2 * i] = i2fp(left);
    buffer[2 * i + 1] = i2fp(right);
  }

  return true;
}
