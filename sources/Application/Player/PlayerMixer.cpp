/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "PlayerMixer.h"
#include "Application/Instruments/SampleInstrument.h"
#include "Application/Mixer/MixerService.h"
#include "Application/Model/Mixer.h"
#include "Application/Utils/char.h"
#include "Application/Utils/fixed.h"
#include "Services/Midi/MidiService.h"
#include "SyncMaster.h"
#include "System/Console/Trace.h"
#include "System/System/System.h"
#include <cstdint>
#include <math.h>
#include <stdlib.h>

PlayerMixer::PlayerMixer() {

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

bool PlayerMixer::Init(Project *project) {

  MixerService *ms = MixerService::GetInstance();
  if (!ms->Init()) {
    return false;
  }

  AudioMixer *audioMixer = ms->GetMixBus(STREAM_MIX_BUS);
  audioMixer->AddModule(fileStreamer_);

  project_ = project;

  // Add the record mixer
  audioMixer->AddModule(recordStreamer_);

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

void PlayerMixer::Close() {

  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
    channel_[i]->Reset();
  }

  MixerService *ms = MixerService::GetInstance();
  ms->Close();
}

bool PlayerMixer::Start() {
  MixerService *ms = MixerService::GetInstance();
  ms->AddObserver(*this);

  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
    notes_[i] = NO_NOTE;
  };

  return ms->Start();
};

void PlayerMixer::Stop() {
  MixerService *ms = MixerService::GetInstance();
  ms->Stop();
  ms->RemoveObserver(*this);
};

void PlayerMixer::StartChannel(int channel) {
  isChannelPlaying_[channel] = true;
};

void PlayerMixer::StopChannel(int channel) {

  StopInstrument(channel);
  isChannelPlaying_[channel] = false;
};

bool PlayerMixer::IsChannelPlaying(int channel) {
  return isChannelPlaying_[channel];
};

I_Instrument *PlayerMixer::GetLastInstrument(int channel) {
  return lastInstrument_[channel];
};

stereosample PlayerMixer::GetMasterOutLevel() {
  MixerService *ms = MixerService::GetInstance();
  return ms->GetMasterBus()->GetMixerLevels();
}

etl::array<stereosample, SONG_CHANNEL_COUNT> *PlayerMixer::GetMixerLevels() {
  MixerService *ms = MixerService::GetInstance();

  // Get the current mixer levels from each bus
  for (int i = 0; i < 8; i++) {
    AudioMixer *audioMixer = ms->GetMixBus(i);
    mixerLevels_[i] = audioMixer->GetMixerLevels();
  }

  return &mixerLevels_;
}

void PlayerMixer::Update(Observable &o, I_ObservableData *d) {

  // Notifies the player so that pattern data is processed

  SetChanged();
  NotifyObservers();

  MixerService *ms = MixerService::GetInstance();
  ms->SetMasterVolume(project_->GetMasterVolume());
};

void PlayerMixer::StartInstrument(int channel, I_Instrument *instrument,
                                  unsigned char note, bool newInstrument) {
  channel_[channel]->StartInstrument(instrument, note, newInstrument);
  lastInstrument_[channel] = instrument;
  notes_[channel] = note;
};

void PlayerMixer::StopInstrument(int channel) {
  channel_[channel]->StopInstrument();
  notes_[channel] = NO_NOTE;
}

I_Instrument *PlayerMixer::GetInstrument(int channel) {
  return channel_[channel]->GetInstrument();
}

int PlayerMixer::GetPlayedBufferPercentage() {
  MixerService *ms = MixerService::GetInstance();
  return ms->GetPlayedBufferPercentage();
};

void PlayerMixer::SetChannelMute(int channel, bool mode) {
  channel_[channel]->SetMute(mode);
}

bool PlayerMixer::IsChannelMuted(int channel) {
  return channel_[channel]->IsMuted();
}

void PlayerMixer::StartStreaming(const char *name, int startSample) {
  fileStreamer_.Start(name, startSample);
};

void PlayerMixer::StartLoopingStreaming(const char *name) {
  fileStreamer_.Start(name, 0, true);
};

void PlayerMixer::StopStreaming() { fileStreamer_.Stop(); };

void PlayerMixer::StartRecordStreaming(uint16_t *srcBuffer, uint32_t size,
                                       bool stereo) {
  recordStreamer_.Start(srcBuffer, size, stereo);
};

void PlayerMixer::StopRecordStreaming() { recordStreamer_.Stop(); };

bool PlayerMixer::IsPlaying() { return fileStreamer_.IsPlaying(); }

void PlayerMixer::OnPlayerStart(MixerServiceMode msmMode) {
  MixerService *ms = MixerService::GetInstance();
  ms->OnPlayerStart(msmMode);
}

void PlayerMixer::OnPlayerStop() {
  MixerService *ms = MixerService::GetInstance();
  ms->OnPlayerStop();
}

static char noteBuffer[5];

static bool shouldShowSlice(int channel, uint8_t &sliceIndex,
                            I_Instrument *instrument,
                            const unsigned char *notes) {
  if (!instrument || instrument->GetType() != IT_SAMPLE) {
    return false;
  }
  if (notes[channel] == 0xFF) {
    return false;
  }
  SampleInstrument *sampleInstr = static_cast<SampleInstrument *>(instrument);
  if (!sampleInstr->ShouldDisplaySliceForNote(notes[channel])) {
    return false;
  }
  sliceIndex =
      static_cast<uint8_t>(notes[channel] - SampleInstrument::SliceNoteBase);
  return true;
}

int PlayerMixer::GetChannelNote(int channel) { return notes_[channel]; }

const char *PlayerMixer::GetPlayedNote(int channel) {

  if (notes_[channel] <= HIGHEST_NOTE) {
    note2visualizer(notes_[channel], noteBuffer);
    return noteBuffer;
  }
  return "  ";
};

const char *PlayerMixer::GetPlayedOctive(int channel) {
  if (notes_[channel] <= HIGHEST_NOTE) {
    if (!IsChannelMuted(channel)) {
      oct2visualizer(notes_[channel], noteBuffer);
      return noteBuffer;
    } else {
      return "--";
    }
  }
  return "  ";
};

bool PlayerMixer::GetPlayedSliceIndex(int channel, uint8_t &sliceIndex) {
  return shouldShowSlice(channel, sliceIndex, lastInstrument_[channel], notes_);
}

AudioOut *PlayerMixer::GetAudioOut() {
  MixerService *ms = MixerService::GetInstance();
  return ms->GetAudioOut();
};

void PlayerMixer::Lock() {
  MixerService *ms = MixerService::GetInstance();
  ms->Lock();
};

void PlayerMixer::Unlock() {
  MixerService *ms = MixerService::GetInstance();
  ms->Unlock();
};
