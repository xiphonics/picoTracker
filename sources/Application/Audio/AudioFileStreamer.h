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
  char name_[128];
  bool newPath_;
  WavFile *wav_;
  int position_;

private:
  Project *project_;

public:
  void SetProject(Project *project) { project_ = project; }
};

#endif
