#include "MixerService.h"
#include "Application/Audio/DummyAudioOut.h"
#include "Application/Model/Config.h"
#include "Application/Model/Mixer.h"
#include "Services/Audio/Audio.h"
#include "Services/Audio/AudioDriver.h"
#include "Services/Midi/MidiService.h"
#include "System/Console/Trace.h"

MixerService::MixerService() : out_(0), sync_(0) {
  mode_ = MSM_AUDIO;
  const char *render = Config::GetInstance()->GetValue("RENDER");
  if (render) {
    if (!strcmp(render, "FILERT")) {
      mode_ = MSM_FILERT;
    };
    if (!strcmp(render, "FILE")) {
      mode_ = MSM_FILE;
    };
    if (!strcmp(render, "FILESPLIT")) {
      mode_ = MSM_FILESPLIT;
    };
    if (!strcmp(render, "FILESPLITRT")) {
      mode_ = MSM_FILESPLITRT;
    };
  };
};

MixerService::~MixerService(){};

bool MixerService::Init() {

  out_ = 0;

  // Create the output depending on rendering mode

  switch (mode_) {
  case MSM_FILE:
  case MSM_FILESPLIT:
    out_ = new DummyAudioOut();
    break;
  default:
    Audio *audio = Audio::GetInstance();
    out_ = audio->GetFirst();
    break;
  }

  bool result = false;

  for (int i = 0; i < MAX_BUS_COUNT; i++) {
    master_.Insert(bus_[i]);
  }

  if (out_) {

    result = out_->Init();
    if (result) {
      out_->Insert(master_);
    }

    switch (mode_) {
    case MSM_AUDIO:
      break;
    case MSM_FILERT:
    case MSM_FILE:
      out_->SetFileRenderer("project:mixdown.wav");
      break;
    case MSM_FILESPLITRT:
    case MSM_FILESPLIT:
      for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
        char buffer[1024];
        sprintf(buffer, "project:channel%d.wav", i);
        bus_[i].SetFileRenderer(buffer);
      }
      break;
    }
    out_->AddObserver(*MidiService::GetInstance());
  }

#ifndef PICOBUILD
  sync_ = SDL_CreateMutex();
#else
  mutex_init(sync_);
#endif
  NAssert(sync_);

  if (result) {
    Trace::Debug("Out initialized");
  } else {
    Trace::Debug("Failed to get output");
  }
  return (result);
};

void MixerService::Close() {
  if (out_) {
    out_->RemoveObserver(*MidiService::GetInstance());
    out_->Close();
    out_->Empty();
    master_.Empty();

    switch (mode_) {
    case MSM_FILE:
    case MSM_FILESPLIT:
      SAFE_DELETE(out_);
      break;
    default:
      break;
    }
    switch (mode_) {
    case MSM_FILESPLITRT:
    case MSM_FILESPLIT:
      break;
    default:
      break;
    }
  }
  for (int i = 0; i < MAX_BUS_COUNT; i++) {
    bus_[i].Empty();
  }
  out_ = 0;
#ifndef PICOBUILD
  SDL_DestroyMutex(sync_);
  sync_ = 0;
#endif
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

bool MixerService::Clipped() { return out_->Clipped(); };

void MixerService::SetMasterVolume(int vol) {
  fixed masterVolume = fp_mul(i2fp(vol), fl2fp(0.01f));

  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
    bus_[i].SetVolume(masterVolume);
  }
};

int MixerService::GetPlayedBufferPercentage() {
  return out_->GetPlayedBufferPercentage();
}

int MixerService::GetAudioPeakL() { return out_->GetLastPeakL(); };

int MixerService::GetAudioPeakR() { return out_->GetLastPeakR(); };

void MixerService::toggleRendering(bool enable) {
  switch (mode_) {
  case MSM_AUDIO:
    break;
  case MSM_FILERT:
  case MSM_FILE:
    out_->EnableRendering(enable);
    break;
  case MSM_FILESPLITRT:
  case MSM_FILESPLIT:
    for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
      bus_[i].EnableRendering(enable);
    };
    break;
  }
}

void MixerService::OnPlayerStart() { toggleRendering(true); };

void MixerService::OnPlayerStop() { toggleRendering(false); };

void MixerService::Execute(FourCC id, float value) {
  if (value > 0.5) {
    Audio *audio = Audio::GetInstance();
    int volume = audio->GetMixerVolume();
    switch (id) {
    case TRIG_VOLUME_INCREASE:
      if (volume < 100)
        volume += 1;
      break;
    case TRIG_VOLUME_DECREASE:
      if (volume > 0)
        volume -= 1;
      break;
    };
    audio->SetMixerVolume(volume);
  };
}

AudioOut *MixerService::GetAudioOut() { return out_; };

void MixerService::Lock() {
  if (sync_)
#ifndef PICOBUILD
    SDL_LockMutex(sync_);
#else
    mutex_enter_blocking(sync_);
#endif
}

void MixerService::Unlock() {
  if (sync_)
#ifndef PICOBUILD
    SDL_UnlockMutex(sync_);
#else
    mutex_exit(sync_);
#endif
}
