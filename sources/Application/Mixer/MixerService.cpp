#include "MixerService.h"
#include "Application/Model/Config.h"
#include "Application/Model/Mixer.h"
#include "Application/Model/Project.h"
#include "Application/Utils/char.h"
#include "Services/Audio/Audio.h"
#include "Services/Audio/AudioDriver.h"
#include "Services/Midi/MidiService.h"
#include "System/Console/Trace.h"
#include "System/System/System.h"
#include <nanoprintf.h>

MixerService::MixerService() : out_(0), sync_(0){};

MixerService::~MixerService(){};

bool MixerService::Init() {
  out_ = 0;
  // Create the output depending on rendering mode
  Audio *audio = Audio::GetInstance();
  out_ = audio->GetFirst();

  bool result = false;

  char buffer[5];
  for (int i = 0; i < MAX_BUS_COUNT; i++) {
    hex2char(i, buffer);
    bus_[i].SetName(etl::string<12>(buffer));
    master_.Insert(bus_[i]);
    master_.SetName("Master");
  }

  if (out_) {
    result = out_->Init();
    if (result) {
      out_->Insert(master_);
    }

    char path[30 + MAX_PROJECT_NAME_LENGTH];
    const char *projectname = Project::ProjectNameGlobal.c_str();

    out_->AddObserver(*MidiService::GetInstance());
    npf_snprintf(path, sizeof(path), "/renders/%s-mixdown.wav", projectname);
    out_->SetFileRenderer(path);
    for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
      npf_snprintf(path, sizeof(path), "/renders/%s-channel%d.wav", projectname,
                   i);
      bus_[i].SetFileRenderer(path);
    }
  }

  mutex_init(sync_);
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

    SAFE_DELETE(out_);
  }
  for (int i = 0; i < MAX_BUS_COUNT; i++) {
    bus_[i].Empty();
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

void MixerService::SetMasterVolume(int vol) {
  fixed masterVolume = fp_mul(i2fp(vol), fl2fp(0.01f));

  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
    bus_[i].SetVolume(masterVolume);
  }
};

int MixerService::GetPlayedBufferPercentage() {
  return out_->GetPlayedBufferPercentage();
}

void MixerService::setRenderingMode(MixerServiceMode mode) {
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

void MixerService::Lock() {
  if (sync_)
    mutex_enter_blocking(sync_);
}

void MixerService::Unlock() {
  if (sync_) {
    mutex_exit(sync_);
  }
}