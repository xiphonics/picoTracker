/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _COMMAND_LIST_H_
#define _COMMAND_LIST_H_

#include "Foundation/Types/Types.h"

class CommandList {
public:
  static TrackerCommand GetNext(TrackerCommand current);
  static TrackerCommand GetPrev(TrackerCommand current);
  static TrackerCommand GetNextAlpha(TrackerCommand current);
  static TrackerCommand GetPrevAlpha(TrackerCommand current);

  // Applies command-specific range limits to parameter values
  // Currently handles:
  // - VEL: Ensures MIDI velocity values don't exceed 127 (0x7F)
  // Can be extended to handle other commands in the future
  // Returns the range-limited parameter value
  static ushort RangeLimitCommandParam(TrackerCommand command,
                                       ushort paramValue);
};
#endif
