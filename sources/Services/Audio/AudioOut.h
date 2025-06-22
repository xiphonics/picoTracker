/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _AUDIO_OUT_H_
#define _AUDIO_OUT_H_

#include "Application/Instruments/WavFileWriter.h"
#include "AudioMixer.h"
#include "Foundation/Observable.h"

class AudioDriver;

#define MIX_BUFFER_SIZE (MAX_SAMPLE_COUNT * 2)

class AudioOut : public AudioMixer, public Observable {
public:
  AudioOut();
  virtual ~AudioOut();
  virtual bool Init() = 0;
  virtual void Close() = 0;
  virtual bool Start() = 0;
  virtual void Stop() = 0;

  //       virtual void SetMasterVolume(int vol)=0 ;

  virtual void Trigger() = 0;

  virtual int GetPlayedBufferPercentage() = 0;

  virtual std::string GetAudioAPI() = 0;
  virtual std::string GetAudioDevice() = 0;
  virtual int GetAudioBufferSize() = 0;
  virtual int GetAudioRequestedBufferSize() = 0;
  virtual int GetAudioPreBufferCount() = 0;

  virtual double GetStreamTime() = 0;

protected:
  // write here the part that gets the float sample size
  // and computes accumulated integer count

  int getPlaySampleCount();

private:
  float sampleOffset_;
};
#endif
