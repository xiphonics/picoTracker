#ifndef _AUDIO_FILE_STREAMER_H_
#define _AUDIO_FILE_STREAMER_H_

#include "Application/Instruments/WavFile.h"
#include "Application/Model/Project.h"
#include "Services/Audio/AudioModule.h"

enum AudioFileStreamerMode { AFSM_STOPPED, AFSM_PLAYING };

class AudioFileStreamer : public AudioModule {
public:
  AudioFileStreamer();
  virtual ~AudioFileStreamer();
  virtual bool Render(fixed *buffer, int samplecount);
  bool Start(char *name);
  void Stop();
  bool IsPlaying();

protected:
  AudioFileStreamerMode mode_;
  char name_[256];
  bool newPath_;
  WavFile *wav_;
  float position_;
  Project *project_;

  // Sample rate conversion
  int fileSampleRate_;
  int systemSampleRate_;
  fixed fpSpeed_; // Fixed-point speed factor for sample rate conversion

public:
  void SetProject(Project *project) { project_ = project; }
};

#endif