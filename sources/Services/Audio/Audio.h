/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _AUDIO_H_
#define _AUDIO_H_

#include "AudioOut.h"
#include "AudioSettings.h"
#include "Foundation/T_Factory.h"
#include "Foundation/T_SimpleList.h"

class Audio : public T_Factory<Audio>, public T_SimpleList<AudioOut> {
public:
  Audio(AudioSettings &settings);
  virtual ~Audio();
  virtual void Init() = 0;
  virtual void Close() = 0;
  virtual int GetSampleRate() { return 44100; };
  virtual int GetMixerVolume() { return 100; };
  virtual void SetMixerVolume(int volume){};

  const char *GetAudioAPI();
  const char *GetAudioDevice();
  int GetAudioBufferSize();
  int GetAudioPreBufferCount();

protected:
  AudioSettings settings_;

private:
  std::string audioAPI_;
  std::string audioDevice_;
  int audioBufferSize_;
  int preBufferCount_;
};
#endif
