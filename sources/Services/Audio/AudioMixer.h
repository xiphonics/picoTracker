/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _AUDIO_MIXER_H_
#define _AUDIO_MIXER_H_

#include "Application/Instruments/WavFileWriter.h"
#include "Application/Player/Reverb.h"
#include "AudioModule.h"
#include "Externals/etl/include/etl/string.h"
#include "Foundation/T_SimpleList.h"
#include "Services/Audio/AudioDriver.h" // for MAX_SAMPLE_COUNT
#include <algorithm>

class AudioMixer : public AudioModule, public T_SimpleList<AudioModule> {
public:
  AudioMixer(const char *name);
  virtual ~AudioMixer();
  virtual bool Render(fixed *buffer, int samplecount);
  void SetFileRenderer(const char *path);
  void EnableRendering(bool enable);
  void SetVolume(fixed volume);
  void SetName(etl::string<12> name) { name_ = name; };
  void EnableReverb(bool enable);
  void SetReverbWet(fixed wet);
  void ClearReverb();

  stereosample GetMixerLevels() { return avgMixerLevel_; }

private:
  bool enableRendering_;
  std::string renderPath_;
  WavFileWriter *writer_;
  fixed volume_;
  etl::string<12> name_;

  // hold the avg volume of a buffer worth of samples for each audiomodule in
  // the mix
  stereosample avgMixerLevel_ = 0;

  static Reverb2 reverb_;
  bool reverb_enabled_;
  fixed reverb_wet_;

  static fixed renderBuffer_[MAX_SAMPLE_COUNT * 2];

  void MasterMixRender(fixed *master_mix_buffer, size_t buffer_size);
};
#endif
