/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#ifndef _ADVAUDIO_DRIVER_H_
#define _ADVAUDIO_DRIVER_H_

#include "Foundation/T_Singleton.h"
#include "Services/Audio/AudioDriver.h"

#define MINI_BLANK_SIZE 256 // Samples

class advAudioDriver : public AudioDriver {
public:
  advAudioDriver(AudioSettings &settings);
  virtual ~advAudioDriver();

  // Sound implementation
  virtual bool InitDriver();
  virtual void CloseDriver();
  virtual bool StartDriver();
  virtual void StopDriver();
  virtual int GetPlayedBufferPercentage();
  virtual int GetSampleRate() { return 44100; };
  virtual bool Interlaced() { return true; };

  // Additional
  void OnChunkDone();
  void SetVolume(int v);
  int GetVolume();
  virtual double GetStreamTime();
  static void IRQHandler();
  static void BufferNeeded();

private:
  static advAudioDriver *instance_;

  AudioSettings settings_;
  static const uint8_t miniBlank_[MINI_BLANK_SIZE];

  uint32_t startTime_;
};
#endif
