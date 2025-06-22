/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _PICOTRACKERAUDIO_H_
#define _PICOTRACKERAUDIO_H_

#include "Services/Audio/Audio.h"

class picoTrackerAudio : public Audio {
public:
  picoTrackerAudio(AudioSettings &hints);
  ~picoTrackerAudio();
  virtual void Init();
  virtual void Close();
  virtual int GetMixerVolume();
  virtual void SetMixerVolume(int volume);

private:
  AudioSettings hints_;
};
#endif
