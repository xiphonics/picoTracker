/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _MIDI_CHANNEL_H_
#define _MIDI_CHANNEL_H_

#include "Foundation/Types/Types.h"
#include "Services/Controllers/Channel.h"

enum MidiControllerType {
  MCT_NONE,
  MCT_HIRES,
  MCT_2_COMP,
  MCT_SIGNED_BIT,
  MCT_SIGNED_BIT_2,
  MCT_BIN_OFFSET
};

class MidiChannel : public Channel {
public:
  MidiChannel(const char *name);
  virtual ~MidiChannel();
  void SetControllerType(MidiControllerType type);
  MidiControllerType GetControllerType();
  void SetToggle(bool toggle);
  bool IsToggle();
  void SetCircular(bool circular);
  bool IsCircular();
  void SetHiRes(bool hires);
  bool IsHiRes();
  void SetRange(int range);
  int GetRange();

private:
  MidiControllerType controllerType_;
  bool toggle_;
  bool circular_;
  bool hiRes_;
  int lastVal_;
  int range_;
};
#endif