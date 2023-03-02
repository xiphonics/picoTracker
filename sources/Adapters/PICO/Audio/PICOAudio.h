#ifndef _PICOAUDIO_H_
#define _PICOAUDIO_H_

#include "Services/Audio/Audio.h"

class PICOAudio : public Audio {
public:
  PICOAudio(AudioSettings &hints);
  ~PICOAudio();
  virtual void Init();
  virtual void Close();
  virtual int GetMixerVolume();
  virtual void SetMixerVolume(int volume);

private:
  AudioSettings hints_;
};
#endif
