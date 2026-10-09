/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "TablePlayback.h"
#include "Application/Instruments/CommandList.h"
#include "Application/Instruments/I_Instrument.h"

TablePlayback TablePlayback::playback_[SONG_CHANNEL_COUNT];
TablePlayback TablePlayback::automationPlayback_[SONG_CHANNEL_COUNT];

void TableSaveState::Reset() {
  position_[0] = 0;
  position_[1] = 0;
  position_[2] = 0;
  groove_.groove_ = -1;
  groove_.position_ = 0;
  groove_.ticks_ = 0;
  for (int i = 0; i < TABLE_STEPS; i++) {
    hopCount_[i][0] = 0;
    hopCount_[i][1] = 0;
    hopCount_[i][2] = 0;
  };
};

void TablePlayback::Reset() {
  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
    playback_[i].Init(i);
    automationPlayback_[i].Init(i);
  }
};

TablePlayback &TablePlayback::GetTablePlayback(int channel) {
  NAssert((channel >= 0) && (channel < SONG_CHANNEL_COUNT));
  return playback_[channel];
}

TablePlayback &TablePlayback::GetAutomationPlayback(int channel) {
  NAssert((channel >= 0) && (channel < SONG_CHANNEL_COUNT));
  return automationPlayback_[channel];
}

void TablePlayback::Init(int channel) {
  channel_ = channel;
  table_ = 0;
  position_[0] = 0;
  position_[1] = 0;
  position_[2] = 0;
  previous_[0] = -1;
  previous_[1] = -1;
  previous_[2] = -1;

  hopped_[0] = false;
  hopped_[1] = false;
  hopped_[2] = false;

  for (int i = 0; i < TABLE_STEPS; i++) {
    hopCount_[i][0] = 0;
    hopCount_[i][1] = 0;
    hopCount_[i][2] = 0;
  };
  instrument_ = 0;
  groove_.groove_ = -1;
  groove_.position_ = 0;
  groove_.ticks_ = 0;

  automated_ = false;
}

void TablePlayback::Start(I_Instrument *i, Table &table, bool automated) {
  if ((!automated) || (automated_ != automated) || (i != instrument_) ||
      (table_ == 0)) {
    instrument_ = i;
    position_[0] = 0;
    position_[1] = 0;
    position_[2] = 0;
    previous_[0] = -1;
    previous_[1] = -1;
    previous_[2] = -1;

    hopped_[0] = false;
    hopped_[1] = false;
    hopped_[2] = false;

    for (int i = 0; i < TABLE_STEPS; i++) {
      hopCount_[i][0] = 0;
      hopCount_[i][1] = 0;
      hopCount_[i][2] = 0;
    };
    groove_.groove_ = -1;
    groove_.position_ = 0;
    groove_.ticks_ = 0;

    automated_ = automated;
  }
  table_ = &table;
}

void TablePlayback::Stop() {
  table_ = 0;
  position_[0] = 0;
  position_[1] = 0;
  position_[2] = 0;
  previous_[0] = -1;
  previous_[1] = -1;
  previous_[2] = -1;

  hopped_[0] = false;
  hopped_[1] = false;
  hopped_[2] = false;
};

int TablePlayback::GetPlaybackPosition(int i) { return previous_[i]; }

Table *TablePlayback::GetTable() { return table_; };

bool TablePlayback::GetAutomation() { return automated_; };

bool TablePlayback::ProcessPositionCommand(uint8_t row, TrackerCommand command,
                                           ushort param) {
  bool hopped = false;

  // First process any positional command
  switch (command) {
  case TrackerCommand::InstrumentCommandHop: {
    int count = param >> 8;
    if (hopCount_[position_[row]][row] == 0) {
      hopCount_[position_[row]][row] = count;
    } else {
      hopCount_[position_[row]][row]--;
    };
    if ((hopCount_[position_[row]][row] != 0) || (count == 0)) {
      position_[row] = param & 0xF;
      hopped = true;
    } else {
      position_[row] = (position_[row] + 1) % 16;
      hopped = true;
    };
    break;
  }
  default:
    break;
  }

  return hopped;
}

bool TablePlayback::ProcessLocalCommand(TrackerCommand command, ushort param,
                                        TablePlayerChange &tpc) {
  switch (command) {
  case TrackerCommand::InstrumentCommandKill:
    tpc.timeToLive_ = (param & 0xFF) + 1;
    return true;
  case TrackerCommand::InstrumentCommandInstrumentRetrigger:
    tpc.instrRetrigger_ = (param & 0xFF);
    return true;
  case TrackerCommand::InstrumentCommandGroove:
    param = param & 0x1F;
    groove_.groove_ = (unsigned char)param;
    groove_.position_ = 0;
    groove_.ticks_ = 0;
    return true;
  case TrackerCommand::InstrumentCommandStop:
    Stop();
    return true;
  default:
    break;
  }
  return false;
}

void TablePlayback::ProcessStep(TablePlayerChange &tpc, uchar currentNote,
                                PlayerLastActiveCommandState &lastActive) {
  Groove *gs = Groove::GetInstance();

  if (table_ != 0) {
    if (instrument_) {
      // See if groove tells us we need to process a step
      if (groove_.ticks_ == 0) {
        // If automated, restore state
        if (automated_) {
          TableSaveState state;
          instrument_->GetTableState(state);
          memcpy(hopCount_, state.hopCount_,
                 sizeof(uchar) * TABLE_STEPS * TABLE_COLUMNS);
          memcpy(position_, state.position_, sizeof(int) * TABLE_COLUMNS);
          groove_ = state.groove_;
        }

        const TrackerCommand currentCmd[TABLE_COLUMNS] = {
            table_->cmd1_[position_[0]],
            table_->cmd2_[position_[1]],
            table_->cmd3_[position_[2]],
        };
        const ushort currentParam[TABLE_COLUMNS] = {
            table_->param1_[position_[0]],
            table_->param2_[position_[1]],
            table_->param3_[position_[2]],
        };

        // Keep table HOP handling in the positional pass, but allow a RND
        // command immediately to the right to randomize the HOP destination
        // before the jump occurs, or a CHN command to gate it.
        TableRowFxResult fx = ResolveTableRowFx(currentCmd, currentParam,
                                                currentNote, lastActive, true);

        hopped_[0] = (fx.allowCmd[0])
                         ? ProcessPositionCommand(0, fx.cmd[0], fx.param[0])
                         : false;
        hopped_[1] = (fx.allowCmd[1])
                         ? ProcessPositionCommand(1, fx.cmd[1], fx.param[1])
                         : false;
        hopped_[2] = (fx.allowCmd[2])
                         ? ProcessPositionCommand(2, fx.cmd[2], fx.param[2])
                         : false;

        if (hopped_[0] || hopped_[1] || hopped_[2]) {
          const TrackerCommand activeCmd[TABLE_COLUMNS] = {
              table_->cmd1_[position_[0]],
              table_->cmd2_[position_[1]],
              table_->cmd3_[position_[2]],
          };
          const ushort activeParam[TABLE_COLUMNS] = {
              table_->param1_[position_[0]],
              table_->param2_[position_[1]],
              table_->param3_[position_[2]],
          };
          fx = ResolveTableRowFx(activeCmd, activeParam, currentNote,
                                 lastActive, true);
        }

        if (fx.allowNote && fx.note <= HIGHEST_NOTE && fx.note != currentNote) {
          tpc.noteRetrigger_ = fx.note;
        }

        auto dispatchResolved = [&](int column) {
          if (!fx.allowCmd[column] ||
              fx.cmd[column] == TrackerCommand::InstrumentCommandNone ||
              fx.cmd[column] == TrackerCommand::InstrumentCommandChance ||
              fx.cmd[column] == TrackerCommand::InstrumentCommandRandom) {
            return;
          }

          const bool handledLocally =
              ProcessLocalCommand(fx.cmd[column], fx.param[column], tpc);
          if (!handledLocally) {
            instrument_->ProcessCommand(channel_, fx.cmd[column],
                                        fx.param[column]);
          }
          UpdateLastActiveCommandState(lastActive, fx.cmd[column],
                                       fx.param[column]);
        };

        dispatchResolved(0);
        dispatchResolved(1);
        dispatchResolved(2);

        previous_[0] = position_[0];
        previous_[1] = position_[1];
        previous_[2] = position_[2];
      }

      // if groove's end reached, update position

      if (gs->UpdateGroove(groove_, true)) {

        if ((table_->cmd1_[position_[0]] !=
             TrackerCommand::InstrumentCommandHop) ||
            (!hopped_[0])) {
          position_[0] = (position_[0] + 1) % 16;
        }
        if ((table_->cmd2_[position_[1]] !=
             TrackerCommand::InstrumentCommandHop) ||
            (!hopped_[1])) {
          position_[1] = (position_[1] + 1) % 16;
        }
        if ((table_->cmd3_[position_[2]] !=
             TrackerCommand::InstrumentCommandHop) ||
            (!hopped_[2])) {
          position_[2] = (position_[2] + 1) % 16;
        }

        hopped_[0] = false;
        hopped_[1] = false;
        hopped_[2] = false;

        if (automated_) {
          TableSaveState state;
          memcpy(state.hopCount_, hopCount_,
                 sizeof(uchar) * TABLE_STEPS * TABLE_COLUMNS);
          memcpy(state.position_, position_, sizeof(int) * TABLE_COLUMNS);
          state.groove_ = groove_;
          instrument_->SetTableState(state);
        }
      }
    }
  }
}
