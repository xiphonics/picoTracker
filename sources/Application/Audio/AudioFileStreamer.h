/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

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
  // Automatically detects single cycle waveforms
  bool Start(const char *name, int startSample = 0);
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

  // For matching oscillator mode in SampleInstrument
  bool useReferencePitch_; // Whether to use the reference pitch instead of
                           // sample rate ratio
  float referencePitch_;   // Reference pitch in Hz (C3 = 130.81 Hz)

public:
  void SetProject(Project *project) { project_ = project; }

  // Single cycle waveform methods
  bool StartLooping(const char *name);
  void StopLooping() { mode_ = AFSM_STOPPED; }
};

#endif