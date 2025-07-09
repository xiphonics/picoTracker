
#include "advAudio.h"
#include "Services/Audio/AudioOutDriver.h"
#include "System/Console/Trace.h"
#include "advAudioDriver.h"

advAudio::advAudio(AudioSettings &hints) : Audio(hints) { hints_ = hints; }

advAudio::~advAudio() {}

void advAudio::Init() {
  AudioSettings settings;
  settings.audioAPI_ = GetAudioAPI();

  settings.bufferSize_ = GetAudioBufferSize();
  settings.preBufferCount_ = GetAudioPreBufferCount();

  __attribute__((
      section(".DATA_RAM"))) static char audioDriver[sizeof(advAudioDriver)];
  advAudioDriver *drv = new (audioDriver) advAudioDriver(settings);
  __attribute__((
      section(".DATA_RAM"))) static char audioOutDriver[sizeof(AudioOutDriver)];
  AudioOutDriver *out = new (audioOutDriver) AudioOutDriver(*drv);
  Insert(out);
};

void advAudio::Close() {
  for (Begin(); !IsDone(); Next()) {
    AudioOut &current = CurrentItem();
    current.Close();
  }
};

void advAudio::SetMixerVolume(int v) {
  AudioOutDriver *out = (AudioOutDriver *)GetFirst();
  if (out) {
    advAudioDriver *drv = (advAudioDriver *)out->GetDriver();
    drv->SetVolume(v);
  }
}

int advAudio::GetMixerVolume() {
  AudioOutDriver *out = (AudioOutDriver *)GetFirst();
  if (out) {
    advAudioDriver *drv = (advAudioDriver *)out->GetDriver();
    return drv->GetVolume();
  }
  return 0;
}
