/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _RECORD_STREAMER_H_
#define _RECORD_STREAMER_H_

#include "Services/Audio/AudioModule.h"

enum RecordStreamerMode { RSM_STOPPED, RSM_PLAYING };

class RecordStreamer : public AudioModule {
public:
  RecordStreamer();
  virtual ~RecordStreamer();
  virtual bool Render(fixed *buffer, int samplecount);
  bool Start(uint16_t *srcBuffer, uint32_t size, bool stereo);
  void Stop();
  bool IsPlaying();

protected:
  uint16_t *srcBuffer_;
  uint32_t bufferSize_;
  bool stereo_;
  RecordStreamerMode mode_;
  uint32_t srcBufferPos_;
};

#endif
