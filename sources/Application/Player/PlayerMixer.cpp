/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "PlayerMixer.h"
#include "Application/Mixer/MixerService.h"
#include "Application/Model/Mixer.h"
#include "Application/Utils/char.h"
#include "Application/Utils/fixed.h"
#include "Services/Midi/MidiService.h"
#include "SyncMaster.h"
#include "System/Console/Trace.h"
#include "System/System/System.h"
#include <math.h>
#include <stdlib.h>

PlayerMixerBase::PlayerMixerBase() {

  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
    lastInstrument_[i] = 0;
  };

  alignas(PlayerChannel) static char
      playerChannelMemBuf[sizeof(PlayerChannel) * SONG_CHANNEL_COUNT];
  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
    channel_[i] =
        new (playerChannelMemBuf + i * sizeof(PlayerChannel)) PlayerChannel(i);
  }
}

bool PlayerMixerBase::Init(Project *project) {

  MixerService *ms = MixerService::GetInstance();
  if (!ms->Init()) {
    return false;
  }

  AudioMixer *audioMixer = ms->GetMixBus(STREAM_MIX_BUS);
  audioMixer->Insert(fileStreamer_);

  project_ = project;

  // Add the record mixer
  audioMixer->Insert(recordStreamer_);

  // Init states
  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
    lastInstrument_[i] = 0;
  };

  // Setup mixbus
  Mixer *mixer = Mixer::GetInstance();
  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
    channel_[i]->SetMixBus(mixer->GetBus(i));
  }

  // streamer need access to project to get current volume
  fileStreamer_.SetProject(project);

  return true;
};

void PlayerMixerBase::Close() {

  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
    channel_[i]->Reset();
  }

  MixerService *ms = MixerService::GetInstance();
  ms->Close();
}

bool PlayerMixerBase::Start() {
  MixerService *ms = MixerService::GetInstance();
  ms->AddObserver(*this);

  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
    notes_[i] = 0xFF;
  };

  return ms->Start();
};

void PlayerMixerBase::Stop() {
  MixerService *ms = MixerService::GetInstance();
  ms->Stop();
  ms->RemoveObserver(*this);
};

void PlayerMixerBase::StartChannel(int channel) {
  isChannelPlaying_[channel] = true;
};

void PlayerMixerBase::StopChannel(int channel) {

  StopInstrument(channel);
  isChannelPlaying_[channel] = false;
};

bool PlayerMixerBase::IsChannelPlaying(int channel) {
  return isChannelPlaying_[channel];
};

I_Instrument *PlayerMixerBase::GetLastInstrument(int channel) {
  return lastInstrument_[channel];
};

stereosample PlayerMixerBase::GetMasterOutLevel() {
  MixerService *ms = MixerService::GetInstance();
  return ms->GetMasterBus()->GetMixerLevels();
}

etl::array<stereosample, SONG_CHANNEL_COUNT> *
PlayerMixerBase::GetMixerLevels() {
  MixerService *ms = MixerService::GetInstance();

  // Get the current mixer levels from each bus
  for (int i = 0; i < 8; i++) {
    AudioMixer *audioMixer = ms->GetMixBus(i);
    mixerLevels_[i] = audioMixer->GetMixerLevels();
  }

  return &mixerLevels_;
}

void PlayerMixerBase::Update(Observable &o, I_ObservableData *d) {

  // Notifies the player so that pattern data is processed

  SetChanged();
  NotifyObservers();

  // Transfer the mixer data
  //     out_->SetMasterVolume(project_->GetMasterVolume()) ;
  MixerService *ms = MixerService::GetInstance();
  ms->SetMasterVolume(project_->GetMasterVolume());
};

void PlayerMixerBase::StartInstrument(int channel, I_Instrument *instrument,
                                      unsigned char note, bool newInstrument) {
  channel_[channel]->StartInstrument(instrument, note, newInstrument);
  lastInstrument_[channel] = instrument;
  notes_[channel] = note;
};

void PlayerMixerBase::StopInstrument(int channel) {
  channel_[channel]->StopInstrument();
  notes_[channel] = 0xFF;
}

I_Instrument *PlayerMixerBase::GetInstrument(int channel) {
  return channel_[channel]->GetInstrument();
}

int PlayerMixerBase::GetPlayedBufferPercentage() {
  MixerService *ms = MixerService::GetInstance();
  return ms->GetPlayedBufferPercentage();
};

void PlayerMixerBase::SetChannelMute(int channel, bool mode) {
  channel_[channel]->SetMute(mode);
}

bool PlayerMixerBase::IsChannelMuted(int channel) {
  return channel_[channel]->IsMuted();
}

void PlayerMixerBase::StartStreaming(const char *name, int startSample) {
  fileStreamer_.Start(name, startSample);
};

void PlayerMixerBase::StartLoopingStreaming(const char *name) {
  fileStreamer_.Start(name, 0, true);
};

void PlayerMixerBase::StopStreaming() { fileStreamer_.Stop(); };

void PlayerMixerBase::StartRecordStreaming(uint16_t *srcBuffer, uint32_t size,
                                           bool stereo) {
  recordStreamer_.Start(srcBuffer, size, stereo);
};

void PlayerMixerBase::StopRecordStreaming() { recordStreamer_.Stop(); };

bool PlayerMixerBase::IsPlaying() { return fileStreamer_.IsPlaying(); }

void PlayerMixerBase::OnPlayerStart(MixerServiceMode msmMode) {
  MixerService *ms = MixerService::GetInstance();
  ms->OnPlayerStart(msmMode);
}

void PlayerMixerBase::OnPlayerStop() {
  MixerService *ms = MixerService::GetInstance();
  ms->OnPlayerStop();
}

static char noteBuffer[5];

int PlayerMixerBase::GetChannelNote(int channel) { return notes_[channel]; }

const char *PlayerMixerBase::GetPlayedNote(int channel) {

  if (notes_[channel] != 0xFF) {
    note2visualizer(notes_[channel], noteBuffer);
    return noteBuffer;
  }
  return "  ";
};

const char *PlayerMixerBase::GetPlayedOctive(int channel) {
  if (notes_[channel] != 0xFF) {
    if (!IsChannelMuted(channel)) {
      oct2visualizer(notes_[channel], noteBuffer);
      return noteBuffer;
    } else {
      return "--";
    }
  }
  return "  ";
};

AudioOut *PlayerMixerBase::GetAudioOut() {
  MixerService *ms = MixerService::GetInstance();
  return ms->GetAudioOut();
};

void PlayerMixerBase::Lock() {
  MixerService *ms = MixerService::GetInstance();
  ms->Lock();
};

void PlayerMixerBase::Unlock() {
  MixerService *ms = MixerService::GetInstance();
  ms->Unlock();
};
