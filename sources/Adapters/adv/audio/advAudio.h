/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#ifndef _ADVAUDIO_H_
#define _ADVAUDIO_H_

#include "Services/Audio/Audio.h"

class advAudio : public Audio {
public:
  advAudio(AudioSettings &hints);
  ~advAudio();
  virtual void Init();
  virtual void Close();
  virtual int GetMixerVolume();
  virtual void SetMixerVolume(int volume);

private:
  AudioSettings hints_;
};
#endif
