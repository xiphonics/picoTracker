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

#include "Externals/etl/include/etl/singleton.h"

// Provide basic functionalities to compute various
// setting regarding tempo, buffer sizes, ticks

class SyncMasterBase {
public:
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
  // Only allow etl::singleton to construct
  friend class etl::singleton<SyncMasterBase>;
  SyncMasterBase();

  int tempo_;
  int currentSlice_;
  int tableRatio_;
  unsigned int beatCount_;
  float playSampleCount_;
  float tickSampleCount_;
};

using SyncMaster = etl::singleton<SyncMasterBase>;
#endif
