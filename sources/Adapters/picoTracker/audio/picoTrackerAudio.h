#ifndef _PICOTRACKERAUDIO_H_
#define _PICOTRACKERAUDIO_H_

#include "Services/Audio/Audio.h"

class picoTrackerAudio : public Audio {
public:
  picoTrackerAudio(AudioSettings &hints);
  ~picoTrackerAudio();
  virtual void Init();
  virtual void Close();
  virtual int GetMixerVolume();
  virtual void SetMixerVolume(int volume);

private:
  AudioSettings hints_;
};
#endif
