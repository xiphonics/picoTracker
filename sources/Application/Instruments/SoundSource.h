/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _SOUND_SOURCE_H_
#define _SOUND_SOURCE_H_

class SoundSource {
public:
  SoundSource(){};
  virtual ~SoundSource(){};
  virtual int GetLoopStart(int note) { return -1; };
  virtual int GetLoopEnd(int note) { return -1; };
  virtual int GetSize(int note) = 0;
  virtual int GetSampleRate(int note) = 0;
  virtual int GetChannelCount(int note) = 0;
  virtual void *GetSampleBuffer(int note) = 0;
  virtual bool IsMulti() = 0;
  virtual int GetRootNote(int note) = 0;
  virtual float GetLengthInSec() = 0;
};

#endif
