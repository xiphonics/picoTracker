/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _TABLE_PLAYBACK_H_
#define _TABLE_PLAYBACK_H_

#include "Application/Model/Groove.h"
#include "Application/Model/Song.h"
#include "Application/Model/Table.h"

class I_Instrument;

class TablePlayerChange {
public:
  int timeToLive_;
  int instrRetrigger_;
};

struct TablePlayback {
public:
  void Init(int i);
  void ProcessStep(TablePlayerChange &tpc);
  bool ProcessLocalCommand(int row, FourCC *commandList, ushort *paramList,
                           TablePlayerChange &tpc);
  void Start(I_Instrument *, Table &, bool automated);
  void Stop();
  int GetPlaybackPosition(int channel);
  Table *GetTable();
  bool GetAutomation();

  static void Reset();
  static TablePlayback &GetTablePlayback(int channel);

private:
  Table *table_;
  int position_[TABLE_COUMNS];
  int previous_[TABLE_COUMNS];
  bool hopped_[TABLE_COUMNS];
  I_Instrument *instrument_;
  int channel_;
  bool automated_;
  uchar hopCount_[TABLE_STEPS][TABLE_COUMNS];
  ChannelGroove groove_;

  static TablePlayback playback_[SONG_CHANNEL_COUNT];
};

class TableSaveState {
public:
  void Reset();
  uchar hopCount_[TABLE_STEPS][TABLE_COUMNS];
  int position_[TABLE_COUMNS];
};

#endif
