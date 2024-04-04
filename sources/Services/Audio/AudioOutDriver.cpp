
#include "AudioOutDriver.h"
#include "Application/Player/SyncMaster.h" // Should be installable
#include "Services/Time/TimeService.h"
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
  clipped_ = false;
  sampleCount_ = 0;
  return driver_->Start();
}

void AudioOutDriver::Stop() { driver_->Stop(); }

bool AudioOutDriver::Clipped() { return clipped_; };

short AudioOutDriver::GetLastPeakL() { return lastPeakVolumeL_; };

short AudioOutDriver::GetLastPeakR() { return lastPeakVolumeR_; };

void AudioOutDriver::Trigger() {
  prepareMixBuffers();
  hasSound_ = AudioMixer::Render(primarySoundBuffer_, sampleCount_);
  clipToMix();
  driver_->AddBuffer(mixBuffer_, sampleCount_);
}

void AudioOutDriver::Update(Observable &o, I_ObservableData *d) {
  SetChanged();
  NotifyObservers(d);
}

void AudioOutDriver::prepareMixBuffers() {
  sampleCount_ = getPlaySampleCount();
  clipped_ = false;
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

    fixed v;
    fixed f_32767 = i2fp(32767);
    fixed f_m32768 = i2fp(-32768);

    short sampleVal = 0;
    short peakL = 0;
    short peakR = 0;

    for (int i = 0; i < sampleCount_; i++) {
      // Left
      v = *p++;
      if (v > f_32767) {
        v = f_32767;
        clipped_ = true;
      } else if (v < f_m32768) {
        v = f_m32768;
        clipped_ = true;
      }
      *s1 = short(fp2i(v));
      s1 += offset;

      sampleVal = short(fp2i(v));
      if (sampleVal >= peakL) {
        peakL = sampleVal;
      }

      // Right
      v = *p++;
      if (v > f_32767) {
        v = f_32767;
        clipped_ = true;
      } else if (v < f_m32768) {
        v = f_m32768;
        clipped_ = true;
      }
      *s2 = short(fp2i(v));
      s2 += offset;

      sampleVal = short(fp2i(v));
      if (sampleVal >= peakR) {
        peakR = sampleVal;
      }
    };
    lastPeakVolumeL_ = peakL;
    lastPeakVolumeR_ = peakR;
    peakL = peakR = 0;
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
