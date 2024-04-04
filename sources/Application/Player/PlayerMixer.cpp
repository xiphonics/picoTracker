

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

PlayerMixer::PlayerMixer() {

  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
    lastInstrument_[i] = 0;
  };

  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
    channel_[i] = new PlayerChannel(i);
  }
}

bool PlayerMixer::Init(Project *project) {

  MixerService *ms = MixerService::GetInstance();
  if (!ms->Init()) {
    return false;
  }

  AudioMixer *audioMixer = ms->GetMixBus(STREAM_MIX_BUS);
  audioMixer->Insert(fileStreamer_);

  project_ = project;

  // Init states

  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
    lastInstrument_[i] = 0;
  };

  clipped_ = false;

  // Setup mixbus
  Mixer *mixer = Mixer::GetInstance();
  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
    channel_[i]->SetMixBus(mixer->GetBus(i));
  }

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
    notes_[i] = 0xFF;
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

short PlayerMixer::GetAudioLevelL() { return peakL_; }

short PlayerMixer::GetAudioLevelR() { return peakR_; }

bool PlayerMixer::Clipped() { return clipped_; }

void PlayerMixer::Update(Observable &o, I_ObservableData *d) {

  // Notifies the player so that pattern data is processed

  SetChanged();
  NotifyObservers();

  // Transfer the mixer data
  //     out_->SetMasterVolume(project_->GetMasterVolume()) ;
  MixerService *ms = MixerService::GetInstance();
  ms->SetMasterVolume(project_->GetMasterVolume());
  clipped_ = ms->Clipped();

  peakL_ = ms->GetAudioPeakL();
  peakR_ = ms->GetAudioPeakR();
};

void PlayerMixer::StartInstrument(int channel, I_Instrument *instrument,
                                  unsigned char note, bool newInstrument) {
  channel_[channel]->StartInstrument(instrument, note, newInstrument);
  lastInstrument_[channel] = instrument;
  notes_[channel] = note;
};

void PlayerMixer::StopInstrument(int channel) {
  channel_[channel]->StopInstrument();
  notes_[channel] = 0xFF;
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

void PlayerMixer::StartStreaming(const Path &path) {
  fileStreamer_.Start(path);
};

void PlayerMixer::StopStreaming() { fileStreamer_.Stop(); };

bool PlayerMixer::IsPlaying() { return fileStreamer_.IsPlaying(); }

void PlayerMixer::OnPlayerStart() {
  MixerService *ms = MixerService::GetInstance();
  ms->OnPlayerStart();
}

void PlayerMixer::OnPlayerStop() {
  MixerService *ms = MixerService::GetInstance();
  ms->OnPlayerStop();
}

static char noteBuffer[5];

int PlayerMixer::GetChannelNote(int channel) { return notes_[channel]; }

const char *PlayerMixer::GetPlayedNote(int channel) {

  if (notes_[channel] != 0xFF) {
    note2visualizer(notes_[channel], noteBuffer);
    return noteBuffer;
  }
  return "  ";
};

const char *PlayerMixer::GetPlayedOctive(int channel) {
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
