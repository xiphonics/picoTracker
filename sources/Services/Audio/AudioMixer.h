#ifndef _AUDIO_MIXER_H_
#define _AUDIO_MIXER_H_

#include "Application/Instruments/WavFileWriter.h"
#include "AudioModule.h"
#include "Foundation/T_SimpleList.h"
#include "Services/Audio/AudioDriver.h" // for MAX_SAMPLE_COUNT
#include <string>

class AudioMixer : public AudioModule, public T_SimpleList<AudioModule> {
public:
  AudioMixer(const char *name);
  virtual ~AudioMixer();
  virtual fixed Render(fixed *buffer, int samplecount);
  void SetFileRenderer(const char *path);
  void EnableRendering(bool enable);
  void SetVolume(fixed volume);
  etl::array<fixed, 8> GetMixerLevels() { return avgModuleLevels_; }

private:
  bool enableRendering_;
  std::string renderPath_;
  WavFileWriter *writer_;
  fixed volume_;
  std::string name_;

  // hold the avg volume of a buffer worth of samples for each audiomodule in
  // the mix
  etl::array<fixed, 8> avgModuleLevels_;

  static fixed renderBuffer_[MAX_SAMPLE_COUNT * 2];
};
#endif
