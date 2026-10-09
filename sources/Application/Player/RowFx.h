/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _ROW_FX_H_
#define _ROW_FX_H_

#include "Application/Model/Song.h"
#include "Application/Model/Table.h"
#include "Foundation/Types/Types.h"

class System;

struct PlayerLastActiveCommandState {
  TrackerCommand lastCmd = TrackerCommand::InstrumentCommandNone;
  ushort lastParam = 0;
  bool valid = false;
};

struct RowFxResult {
  bool allowNote = true;
  bool allowCmd1 = true;
  bool allowCmd2 = true;
  uchar note = NO_NOTE;
  TrackerCommand cmd1 = TrackerCommand::InstrumentCommandNone;
  TrackerCommand cmd2 = TrackerCommand::InstrumentCommandNone;
  ushort param1 = 0;
  ushort param2 = 0;
};

struct TableRowFxResult {
  bool allowNote = true;
  uchar note = NO_NOTE;
  bool allowCmd[TABLE_COLUMNS] = {true, true, true};
  TrackerCommand cmd[TABLE_COLUMNS] = {
      TrackerCommand::InstrumentCommandNone,
      TrackerCommand::InstrumentCommandNone,
      TrackerCommand::InstrumentCommandNone,
  };
  ushort param[TABLE_COLUMNS] = {0, 0, 0};
};

RowFxResult ResolveRowFx(uchar note, TrackerCommand cmd1, ushort param1,
                         TrackerCommand cmd2, ushort param2,
                         const PlayerLastActiveCommandState &lastActive,
                         bool includeCommandFx, bool enableNoteChance = true);

bool RollChance(uint8_t amount, System *system);

int32_t ResolveRowHop(TrackerCommand cmd1, ushort param1, TrackerCommand cmd2,
                      ushort param2);

TableRowFxResult
ResolveTableRowFx(const TrackerCommand (&cmd)[TABLE_COLUMNS],
                  const ushort (&param)[TABLE_COLUMNS], uchar currentNote,
                  const PlayerLastActiveCommandState &lastActive,
                  bool includeCommandFx);

void UpdateLastActiveCommandState(PlayerLastActiveCommandState &state,
                                  TrackerCommand cmd, ushort param);

#endif
