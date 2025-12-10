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
#include "AudioModule.h"
#include "Externals/etl/include/etl/string.h"
#include "Foundation/T_SimpleList.h"
#include "Services/Audio/AudioDriver.h" // for MAX_SAMPLE_COUNT
#include "config/StringLimits.h"

class AudioMixer : public AudioModule, public T_SimpleList<AudioModule> {
public:
  AudioMixer(const char *name);
  virtual ~AudioMixer();
  virtual bool Render(fixed *buffer, int samplecount);
  void SetFileRenderer(const char *path);
  void EnableRendering(bool enable);
  void SetVolume(fixed volume);
  void SetName(etl::string<12> name) { name_ = name; };

  stereosample GetMixerLevels() { return peakMixerLevel_; }

private:
  bool enableRendering_;
  etl::string<STRING_AUDIO_RENDER_PATH_MAX> renderPath_;
  WavFileWriter *writer_;
  fixed volume_;
  etl::string<12> name_;

  // hold the avg volume of a buffer worth of samples for each audiomodule in
  // the mix
  stereosample peakMixerLevel_ = 0;

  __attribute__((section(".DTCMRAM")))
  __attribute__((aligned(32))) static fixed renderBuffer_[MAX_SAMPLE_COUNT * 2];
};
#endif
