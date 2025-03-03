#include "MixerService.h"
#include "Application/Model/Config.h"
#include "Application/Model/Mixer.h"
#include "Services/Audio/Audio.h"
#include "Services/Audio/AudioDriver.h"
#include "Services/Midi/MidiService.h"
#include "System/Console/Trace.h"
#include "System/System/System.h"
#include <nanoprintf.h>

MixerService::MixerService() : out_(0), sync_(0) {
  mode_ = MSM_AUDIO;

  // render value is saved as value of enum MixerServiceMode
  mode_ = (MixerServiceMode)Config::GetInstance()->GetValue("RENDER");
};

MixerService::~MixerService(){};

bool MixerService::Init() {

  out_ = 0;

  // Create the output depending on rendering mode

  switch (mode_) {
  case MSM_FILE:
  case MSM_FILESPLIT:
    // This is where dummy audio was initialized for the file rendering use case
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
        char buffer[18];
        npf_snprintf(buffer, sizeof(buffer), "project:channel%d.wav", i);
        bus_[i].SetFileRenderer(buffer);
      }
      break;
    }
    out_->AddObserver(*MidiService::GetInstance());
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
