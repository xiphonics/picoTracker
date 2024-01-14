
#include "picoTrackerAudio.h"
#include "Services/Audio/AudioOutDriver.h"
#include "System/Console/Trace.h"
#include "picoTrackerAudioDriver.h"

picoTrackerAudio::picoTrackerAudio(AudioSettings &hints) : Audio(hints) {
  hints_ = hints;
}

picoTrackerAudio::~picoTrackerAudio() {}

void picoTrackerAudio::Init() {
  AudioSettings settings;
  settings.audioAPI_ = GetAudioAPI();

  settings.bufferSize_ = GetAudioBufferSize();
  settings.preBufferCount_ = GetAudioPreBufferCount();

  picoTrackerAudioDriver *drv = new picoTrackerAudioDriver(settings);
  AudioOutDriver *out = new AudioOutDriver(*drv);
  Insert(out);
};

void picoTrackerAudio::Close() {
  IteratorPtr<AudioOut> it(GetIterator());
  for (it->Begin(); !it->IsDone(); it->Next()) {
    AudioOut &current = it->CurrentItem();
    current.Close();
  }
};

void picoTrackerAudio::SetMixerVolume(int v) {
  AudioOutDriver *out = (AudioOutDriver *)GetFirst();
  if (out) {
    picoTrackerAudioDriver *drv = (picoTrackerAudioDriver *)out->GetDriver();
    drv->SetVolume(v);
  }
}

int picoTrackerAudio::GetMixerVolume() {
  AudioOutDriver *out = (AudioOutDriver *)GetFirst();
  if (out) {
    picoTrackerAudioDriver *drv = (picoTrackerAudioDriver *)out->GetDriver();
    return drv->GetVolume();
  }
  return 0;
}
