/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _GROOVE_H_
#define _GROOVE_H_

#include "Application/Model/Song.h"
#include "Application/Persistency/Persistent.h"
#include "Application/Utils/HexBuffers.h"
#include "Externals/etl/include/etl/singleton.h"

#define MAX_GROOVES 0x20
#define NO_GROOVE_DATA 0xFF

struct ChannelGroove {
  unsigned char groove_;   // selected groove
  unsigned char position_; // step in the groove table
  unsigned char ticks_;    // number of ticks before next step
};

class GrooveBase : Persistent {
public:
  void Reset();
  void Clear();
  void Trigger();
  bool TriggerChannel(int channel);
  void SetGroove(int channel, int groove);
  bool UpdateGroove(ChannelGroove &g, bool reverse);
  void GetChannelData(int channel, int *groove, int *position);
  unsigned char *GetGrooveData(int groove);
  virtual void SaveContent(tinyxml2::XMLPrinter *printer);
  virtual void RestoreContent(PersistencyDocument *doc);

private:
  // Only allow etl::singleton to construct
  friend class etl::singleton<GrooveBase>;
  GrooveBase();

  ChannelGroove channelGroove_[SONG_CHANNEL_COUNT];
  static unsigned char data_[MAX_GROOVES][16];
};

using Groove = etl::singleton<GrooveBase>;
#endif
