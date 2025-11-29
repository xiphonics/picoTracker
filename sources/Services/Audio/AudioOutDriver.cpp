/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "AudioOutDriver.h"
#include "Application/Player/SyncMaster.h" // Should be installable
#include "System/Console/Trace.h"
#include "System/System/System.h"

fixed AudioOutDriver::primarySoundBuffer_[MIX_BUFFER_SIZE];
short AudioOutDriver::mixBuffer_[MIX_BUFFER_SIZE];

AudioOutDriver::AudioOutDriver(AudioDriver &driver) {
  driver_ = &driver;
  driver.AddObserver(*this);
  SetOwnership(false);
}

AudioOutDriver::~AudioOutDriver() {
  driver_->RemoveObserver(*this);
  delete driver_;
};

bool AudioOutDriver::Init() { return driver_->Init(); };

void AudioOutDriver::Close() { driver_->Close(); }

bool AudioOutDriver::Start() {
  sampleCount_ = 0;
  return driver_->Start();
}

void AudioOutDriver::Stop() { driver_->Stop(); }

stereosample AudioOutDriver::GetLastPeakLevels() { return lastPeakVolume_; };

void AudioOutDriver::Trigger() {
  prepareMixBuffers();
  hasSound_ = AudioMixer::Render(primarySoundBuffer_, sampleCount_) > 0;
  clipToMix();
  driver_->AddBuffer(mixBuffer_, sampleCount_);
}

void AudioOutDriver::Update(Observable &o, I_ObservableData *d) {
  SetChanged();
  NotifyObservers(d);
}

void AudioOutDriver::prepareMixBuffers() {
  sampleCount_ = getPlaySampleCount();
};

void AudioOutDriver::clipToMix() {

  bool interlaced = driver_->Interlaced();

  if (!hasSound_) {
    SYS_MEMSET(mixBuffer_, 0, sampleCount_ * 2 * sizeof(short));
  } else {
    short *s1 = mixBuffer_;
    short *s2 = (interlaced) ? s1 + 1 : s1 + sampleCount_;
    int offset = (interlaced) ? 2 : 1;

    fixed *p = primarySoundBuffer_;

    short peakL = 0;
    short peakR = 0;

    for (int i = 0; i < sampleCount_; i++) {
      // Left
      short l = fp2i(*p++);
      *s1 = l;
      s1 += offset;
      
      // Right
      short r = fp2i(*p++);
      *s2 = r;
      s2 += offset;

      // update the level every 32 sample pairs
      if (!(i & 0b11111)) {
        if (l > peakL) peakL = l;
        if (r > peakR) peakR = r;
      }
    }

    lastPeakVolume_ = peakL << 16 | peakR;
  }
};

int AudioOutDriver::GetPlayedBufferPercentage() {
  return driver_->GetPlayedBufferPercentage();
};

AudioDriver *AudioOutDriver::GetDriver() { return driver_; };

std::string AudioOutDriver::GetAudioAPI() {
  AudioSettings as = driver_->GetAudioSettings();
  return as.audioAPI_;
};

std::string AudioOutDriver::GetAudioDevice() {
  AudioSettings as = driver_->GetAudioSettings();
  return as.audioDevice_;
};

int AudioOutDriver::GetAudioBufferSize() {
  AudioSettings as = driver_->GetAudioSettings();
  return as.bufferSize_;
};

int AudioOutDriver::GetAudioRequestedBufferSize() {
  AudioSettings as = driver_->GetAudioSettings();
  return as.bufferSize_;
}

int AudioOutDriver::GetAudioPreBufferCount() {
  AudioSettings as = driver_->GetAudioSettings();
  return as.preBufferCount_;
};
double AudioOutDriver::GetStreamTime() { return driver_->GetStreamTime(); };
