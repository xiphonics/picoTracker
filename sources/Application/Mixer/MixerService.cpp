/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "MixerService.h"
#include "Application/AppWindow.h"
#include "Application/Application.h"
#include "Application/Model/Config.h"
#include "Application/Model/Project.h"
#include "Application/Player/Player.h"
#include "Application/Player/PlayerMixer.h"
#include "Application/Utils/char.h"
#include "Services/Audio/Audio.h"
#include "Services/Audio/AudioDriver.h"
#include "Services/Audio/AudioOut.h"
#include "Services/Midi/MidiService.h"
#include "System/Console/Trace.h"
#include "System/System/System.h"
#include "platform.h"
#include <nanoprintf.h>

MixerService::MixerService() : master_(), sync_(platform_mutex()) {
  out_ = 0;
  project_ = NULL;
  master_.SetName("Master");
};

MixerService::~MixerService(){};

bool MixerService::Init() {
  out_ = 0;
  // Create the output depending on rendering mode
  Audio *audio = Audio::GetInstance();
  out_ = audio->GetFirstOutput();

  bool result = false;

  char buffer[5];
  for (int i = 0; i < MAX_BUS_COUNT; i++) {
    hex2char(i, buffer);
    bus_[i].SetName(etl::string<12>(buffer));
    master_.AddModule(bus_[i]);
    master_.SetName("Master");
  }

  if (out_) {
    result = out_->Init();
    if (result) {
      out_->AddModule(master_);
    }

    out_->AddObserver(*MidiService::GetInstance());
    bool pathConfigured = configureRenderPaths();
    if (!pathConfigured) {
      Trace::Error("[MixerService::Init] Failed to set audio render paths");
      return false;
    }
  }

  if (result) {
    Trace::Debug("[MixerService::Init] Out initialized");
  } else {
    Trace::Error("[MixerService::Init] Failed to get output");
  }
  return (result);
};

void MixerService::Close() {
  if (out_) {
    out_->RemoveObserver(*MidiService::GetInstance());
    out_->Close();
    out_->ClearModules();
    master_.ClearModules();
  }
  for (int i = 0; i < MAX_BUS_COUNT; i++) {
    bus_[i].ClearModules();
  }
  out_ = 0;
};

bool MixerService::Start() {
  MidiService::GetInstance()->Start();
  if (out_) {
    out_->AddObserver(*this);
    out_->Start();
  }
  return true;
};

void MixerService::Stop() {
  MidiService::GetInstance()->Stop();
  if (out_) {
    out_->Stop();
    out_->RemoveObserver(*this);
  }
}

MixBus *MixerService::GetMixBus(int i) { return &(bus_[i]); };

void MixerService::Update(Observable &o, I_ObservableData *d) {

  AudioDriver::Event *event = (AudioDriver::Event *)d;
  if (event->type_ == AudioDriver::Event::ADET_BUFFERNEEDED) {
    Lock();
    SetChanged();
    NotifyObservers();

    out_->Trigger();
    Unlock();
  }
}

// Helper function to convert linear volume (0-99) to non-linear (0.0-1.0) in
// fixed point
fixed MixerService::ToLogVolume(int vol) {
  // Ensure vol is within valid range
  if (vol < 0)
    vol = 0;
  if (vol >= 99) { // for now treating 99 as unity gain
    // Unity gain – treat as a no-op to avoid unnecessary scaling
    return i2fp(1);
  }

  // Convert to fixed point (0-1 range)
  fixed normalizedVol = fp_mul(i2fp(vol), fl2fp(0.01f));

  // Apply quadratic curve for logarithmic-like scaling
  // This gives better control at lower volumes
  return fp_mul(normalizedVol, normalizedVol);
}

void MixerService::SetMasterVolume(int vol) {
  // Apply logarithmic scaling for better volume control
  // vol is 0-99, where 99 is unity gain (1.0)
  fixed masterVolume = ToLogVolume(vol);

  // Set the master bus volume
  master_.SetVolume(masterVolume);

  Player *player = Player::GetInstance();
  Project *project = player ? player->GetProject() : nullptr;

  // Apply channel volumes to individual channel buses
  if (project) {
    for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
      // Get the channel's individual volume (0-99)
      int channelVol = project->GetChannelVolume(i);

      // Convert channel volume to non-linear scale (0.0-1.0)
      fixed channelVolume = ToLogVolume(channelVol);

      // Set the channel volume directly (not multiplied by master volume)
      bus_[i].SetVolume(channelVolume);
      // Trace::Debug("Set channel %d volume to %d", i, channelVol);
    }
  } else {
    // assert and crash
    NAssert(false);
  }
}

int MixerService::GetPlayedBufferPercentage() {
  return out_->GetPlayedBufferPercentage();
}

void MixerService::setRenderingMode(MixerServiceMode mode) {
  if (mode != MSM_AUDIO) {
    // in case proj name changed since last time paths were configured
    bool pathsResult = configureRenderPaths();
    if (!pathsResult) {
      Trace::Error(
          "[MixerService::setRenderingMode] Failed to set render paths");
    }
  }

  switch (mode) {
  case MSM_AUDIO:
    out_->EnableRendering(false);
    for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
      bus_[i].EnableRendering(false);
    };
    break;
  case MSM_FILE:
    out_->EnableRendering(true);
    break;
  case MSM_FILESPLIT:
    for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
      bus_[i].EnableRendering(true);
    };
    break;
  }
}

void MixerService::OnPlayerStart(MixerServiceMode mode) {
  setRenderingMode(mode);
};

void MixerService::OnPlayerStop() {
  // always reset back to audio mode when stopping
  setRenderingMode(MSM_AUDIO);
};

bool MixerService::configureRenderPaths() {
  if (!out_) {
    return false;
  }

  Player *player = Player::GetInstance();
  Project *project = player ? player->GetProject() : nullptr;
  if (!project) {
    Trace::Error("MIXERSERVICE", "Cannot configure render paths, project null");
    return false;
  }

  char projectname[MAX_PROJECT_NAME_LENGTH];
  project->GetProjectName(projectname);

  char path[30 + MAX_PROJECT_NAME_LENGTH];
  npf_snprintf(path, sizeof(path), "/renders/%s-mixdown.wav", projectname);
  out_->SetFileRenderer(path);
  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
    npf_snprintf(path, sizeof(path), "/renders/%s-channel%d.wav", projectname,
                 i);
    bus_[i].SetFileRenderer(path);
  }
  return true;
}

void MixerService::Execute(FourCC id, float value) {
  if (value > 0.5) {
    Audio *audio = Audio::GetInstance();
    int volume = audio->GetMixerVolume();
    switch (id) {
    case FourCC::TrigVolumeIncrease:
      if (volume < 100)
        volume += 1;
      break;
    case FourCC::TrigVolumeDecrease:
      if (volume > 0)
        volume -= 1;
      break;
    };
    audio->SetMixerVolume(volume);
  };
}

AudioOut *MixerService::GetAudioOut() { return out_; };

void MixerService::Lock() { sync_->Lock(); }

void MixerService::Unlock() { sync_->Unlock(); }
