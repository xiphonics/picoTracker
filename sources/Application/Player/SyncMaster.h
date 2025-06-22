/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _SYNC_MASTER_H_
#define _SYNC_MASTER_H_

#include "Foundation/T_Singleton.h"

// Provide basic functionalities to compute various
// setting regarding tempo, buffer sizes, ticks

class SyncMaster : public T_Singleton<SyncMaster> {
public:
  SyncMaster();
  void Start();
  void Stop();
  void SetTempo(int tempo);
  int GetTempo();
  void NextSlice();
  bool MajorSlice();
  bool TableSlice();
  bool MidiSlice();
  float GetPlaySampleCount();
  float GetTickSampleCount();
  int GetTableRatio();
  void SetTableRatio(int ratio);
  unsigned int GetBeatCount();
  float GetTickTime();

private:
  int tempo_;
  int currentSlice_;
  int tableRatio_;
  unsigned int beatCount_;
  float playSampleCount_;
  float tickSampleCount_;
};
#endif
