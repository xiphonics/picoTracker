/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _AUDIO_DRIVER_H_
#define _AUDIO_DRIVER_H_

#include "AudioSettings.h"
#include "Foundation/Observable.h"

#define SOUND_BUFFER_COUNT 2
#define SOUND_BUFFER_MAX 7500
#define MAX_SAMPLE_COUNT 1875

struct AudioBufferData {
  char buffer_[MAX_SAMPLE_COUNT * 2 * sizeof(short)];
  int size_;
  bool empty_;
  void *driverData_;
};

class AudioDriver : public Observable {

public:
  class Event : public I_ObservableData {
  public:
    enum Type { ADET_DRIVERTICK, ADET_BUFFERNEEDED };

    Event(Type type) { type_ = type; };
    Type type_;
  };

public:
  AudioDriver(AudioSettings &settings);
  virtual ~AudioDriver();

  virtual bool Init();
  virtual void Close();
  virtual bool Start();
  virtual void Stop();

  virtual bool InitDriver() = 0;
  virtual void CloseDriver() = 0;
  virtual bool StartDriver() = 0;
  virtual void StopDriver() = 0;

  virtual bool Interlaced() = 0;
  virtual int GetPlayedBufferPercentage() = 0;
  virtual void OnAudioActive(bool active) {}

  virtual double GetStreamTime() = 0; // in secs

  void AddBuffer(short *buffer, int size); // size in samples

  AudioSettings GetAudioSettings();

  void OnNewBufferNeeded();

protected:
  void eatBuffer(void *buffer, int size); // size in bytes
  void onAudioBufferTick();
  bool hasData();
  AudioSettings settings_;

protected:
  bool isPlaying_;
  static AudioBufferData pool_[SOUND_BUFFER_COUNT];
  int poolQueuePosition_;
  int poolPlayPosition_;
  int bufferPos_;
  int bufferSize_;
  bool hasData_;
};
#endif
