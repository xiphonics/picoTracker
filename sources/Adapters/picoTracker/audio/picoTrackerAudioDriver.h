/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _PICOTRACKERAUDIO_DRIVER_H_
#define _PICOTRACKERAUDIO_DRIVER_H_

#include "Foundation/T_Singleton.h"
#include "Services/Audio/AudioDriver.h"

#define MINI_BLANK_SIZE 128 // Samples

class picoTrackerAudioDriver : public AudioDriver {
public:
  picoTrackerAudioDriver(AudioSettings &settings);
  virtual ~picoTrackerAudioDriver();

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
  void SetLineOutLevel(uint8_t level);
  static void UpdateHardwareLineOutLevel(uint8_t level);
  virtual double GetStreamTime();
  static void IRQHandler();
  static void BufferNeeded();

private:
  void ApplyLineOutLevel(uint8_t level);
  uint8_t ClampLineOutLevel(uint8_t level) const;

  static picoTrackerAudioDriver *instance_;

  AudioSettings settings_;
  static const char miniBlank_[MINI_BLANK_SIZE * 2 * sizeof(short)];
  int volume_;
  uint32_t startTime_;
  uint8_t lineOutLevel_;
  bool pioConfigured_;
};
#endif
