#ifndef _AUDIO_FILE_STREAMER_H_
#define _AUDIO_FILE_STREAMER_H_

#include "Application/Instruments/WavFile.h"
#include "Application/Model/Project.h"
#include "Services/Audio/AudioModule.h"

#define SINGLE_CYCLE_MAX_SAMPLE_SIZE 600

enum AudioFileStreamerMode { AFSM_STOPPED, AFSM_PLAYING, AFSM_LOOPING };

class AudioFileStreamer : public AudioModule {
public:
  AudioFileStreamer();
  virtual ~AudioFileStreamer();
  virtual bool Render(fixed *buffer, int samplecount);
  bool Start(char *name); // Automatically detects single cycle waveforms
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

  // Static buffer for single cycle waveforms (max 600 samples in mono from AKWF
  // single cycle format
  static short singleCycleBuffer_[SINGLE_CYCLE_MAX_SAMPLE_SIZE];
  short *singleCycleData_; // Pointer to the current single cycle data

public:
  void SetProject(Project *project) { project_ = project; }

  // Single cycle waveform methods
  bool StartLooping(char *name);
  void StopLooping() { mode_ = AFSM_STOPPED; }
};

#endif