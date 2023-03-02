
#include "PICOAudio.h"
#include "PICOAudioDriver.h"
#include "Services/Audio/AudioOutDriver.h"
#include "System/Console/Trace.h"

PICOAudio::PICOAudio(AudioSettings &hints) : Audio(hints) { hints_ = hints; }

PICOAudio::~PICOAudio() {}

void PICOAudio::Init() {
  AudioSettings settings;
  settings.audioAPI_ = GetAudioAPI();

  settings.bufferSize_ = GetAudioBufferSize();
  settings.preBufferCount_ = GetAudioPreBufferCount();

  PICOAudioDriver *drv = new PICOAudioDriver(settings);
  AudioOutDriver *out = new AudioOutDriver(*drv);
  Insert(out);
};

void PICOAudio::Close() {
  IteratorPtr<AudioOut> it(GetIterator());
  for (it->Begin(); !it->IsDone(); it->Next()) {
    AudioOut &current = it->CurrentItem();
    current.Close();
  }
};

void PICOAudio::SetMixerVolume(int v) {
  AudioOutDriver *out = (AudioOutDriver *)GetFirst();
  if (out) {
    PICOAudioDriver *drv = (PICOAudioDriver *)out->GetDriver();
    drv->SetVolume(v);
  }
}

int PICOAudio::GetMixerVolume() {
  AudioOutDriver *out = (AudioOutDriver *)GetFirst();
  if (out) {
    PICOAudioDriver *drv = (PICOAudioDriver *)out->GetDriver();
    return drv->GetVolume();
  }
  return 0;
}
