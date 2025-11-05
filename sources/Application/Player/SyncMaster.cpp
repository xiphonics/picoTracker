/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "SyncMaster.h"
#include "Services/Audio/Audio.h"

#ifdef WIN32
#define AUDIO_SLICES_PER_STEP 6 // needs to be a multiple of 6 !
#else
#define AUDIO_SLICES_PER_STEP 6 // needs to be a multiple of 6 !
#endif

SyncMasterBase::SyncMasterBase() { tableRatio_ = 1; }

void SyncMasterBase::Start() {
  currentSlice_ = 0;
  beatCount_ = 0;
};

void SyncMasterBase::Stop(){};

void SyncMasterBase::SetTempo(int tempo) {
  tempo_ = tempo;
  int driverRate = Audio::GetInstance()->GetSampleRate();
  playSampleCount_ =
      60.0f * driverRate * 2.0f / tempo_ / 8.0f / float(AUDIO_SLICES_PER_STEP);
  tickSampleCount_ = 60.0f * driverRate * 2.0f / tempo_ / 8.0f /
                     float(AUDIO_SLICES_PER_STEP) * tableRatio_;
};

int SyncMasterBase::GetTempo() { return tempo_; };

void SyncMasterBase::NextSlice() {
  currentSlice_ = (currentSlice_ + 1) % AUDIO_SLICES_PER_STEP;
  if (currentSlice_ == 0) {
    beatCount_++;
  };
};

bool SyncMasterBase::MajorSlice() { return currentSlice_ == 0; };

bool SyncMasterBase::TableSlice() {
  int tableTick = currentSlice_ % (AUDIO_SLICES_PER_STEP / 6 * tableRatio_);
  return tableTick == 0;
};

bool SyncMasterBase::MidiSlice() {
  int midiTick = currentSlice_ % (AUDIO_SLICES_PER_STEP / 6);
  return midiTick == 0;
};

// Returns the number of samples per play slice

float SyncMasterBase::GetPlaySampleCount() { return playSampleCount_; };

// Returns the number of sample per tick
float SyncMasterBase::GetTickSampleCount() { return tickSampleCount_; };

// xx samples per tick
// xx/driverRate seconds
// xx/driverRate*1000 msecs

float SyncMasterBase::GetTickTime() {
  return 60.0f * 2.0f / tempo_ / 8.0f / AUDIO_SLICES_PER_STEP * 1000.0f;
};

void SyncMasterBase::SetTableRatio(int ratio) { tableRatio_ = ratio; }

int SyncMasterBase::GetTableRatio() { return tableRatio_; }

unsigned int SyncMasterBase::GetBeatCount() { return beatCount_; };
