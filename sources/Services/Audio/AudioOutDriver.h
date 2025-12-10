/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _AUDIO_OUT_DRIVER_H_
#define _AUDIO_OUT_DRIVER_H_

#include "Application/Instruments/WavFileWriter.h"
#include "AudioDriver.h"
#include "AudioOut.h"
#include "Foundation/Observable.h"

class AudioDriver;

class AudioOutDriver : public AudioOut, protected I_Observer {
public:
  AudioOutDriver(AudioDriver &);
  virtual ~AudioOutDriver();

  virtual bool Init();
  virtual void Close();
  virtual bool Start();
  virtual void Stop();
  void SetAudioActive(bool active) override;

  virtual void Trigger();

  virtual stereosample GetLastPeakLevels();

  virtual int GetPlayedBufferPercentage();

  AudioDriver *GetDriver();

  virtual std::string GetAudioAPI();
  virtual std::string GetAudioDevice();
  virtual int GetAudioBufferSize();
  virtual int GetAudioRequestedBufferSize();
  virtual int GetAudioPreBufferCount();
  virtual double GetStreamTime();

protected:
  virtual void Update(Observable &o, I_ObservableData *d);

  void prepareMixBuffers();
  void mixToPrimary();
  void clipToMix();

private:
  AudioDriver *driver_;
  bool hasSound_ = false;
  stereosample lastPeakVolume_ = 0;

  __attribute__((section(".DTCMRAM"))) __attribute__((
      aligned(32))) static fixed primarySoundBuffer_[MIX_BUFFER_SIZE];
  __attribute__((section(".DTCMRAM")))
  __attribute__((aligned(32))) static short mixBuffer_[MIX_BUFFER_SIZE];
  int sampleCount_;
};
#endif
