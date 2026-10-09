/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "RowFx.h"
#include "Application/Instruments/CommandList.h"
#include "Application/Model/Project.h"
#include "Application/Model/Song.h"
#include "System/System/System.h"
#include <algorithm>

namespace {

static uint8_t GetRandomByte(System *system) {
  return static_cast<uint8_t>(system->GetRandomNumber() & 0xFF);
}

} // namespace

bool RollChance(uint8_t amount, System *system) {
  if (amount == 0x00) {
    return false;
  }
  if (amount == 0xFF) {
    return true;
  }
  return GetRandomByte(system) <= amount;
}

namespace {

static uint8_t RandomizeValue(uint8_t original, uint8_t amount, uint8_t maximum,
                              System *system) {
  if (amount == 0x00) {
    return original;
  }
  const int random = static_cast<int>(GetRandomByte(system));
  const int delta = ((random - 128) * static_cast<int>(amount)) / 127;
  return static_cast<uint8_t>(std::clamp(static_cast<int>(original) + delta, 0,
                                         static_cast<int>(maximum)));
}

static uint8_t RandomizeByte(uint8_t original, uint8_t amount, System *system) {
  return RandomizeValue(original, amount, 255, system);
}

static uint8_t RandomizeNibble(uint8_t original, uint8_t amount,
                               System *system) {
  return RandomizeValue(original & 0x0F, amount & 0x0F, 15, system);
}

static ushort RandomizeNibbles(ushort originalParam, ushort amount,
                               System *system) {
  ushort randomized = 0;
  for (uint8_t index = 0; index < 4; index++) {
    const uint8_t shift = static_cast<uint8_t>((3 - index) * 4);
    const uint8_t nibble =
        static_cast<uint8_t>((originalParam >> shift) & 0x0F);
    const uint8_t nibbleAmount = static_cast<uint8_t>((amount >> shift) & 0x0F);
    randomized |= static_cast<ushort>(
        RandomizeNibble(nibble, nibbleAmount, system) << shift);
  }
  return randomized;
}

static ushort RandomizeLowNibble(ushort originalParam, uint8_t amount,
                                 System *system) {
  return static_cast<ushort>(RandomizeNibble(
      static_cast<uint8_t>(originalParam & 0x0F), amount, system));
}

static ushort RandomizeLowNibbleWithByteAmountPreservingUpper(
    ushort originalParam, uint8_t amount, System *system) {
  if (amount == 0x00) {
    return originalParam;
  }

  const uint8_t original = static_cast<uint8_t>(originalParam & 0x0F);
  const uint8_t depth = static_cast<uint8_t>((amount * 15 + 127) / 255);
  const uint8_t minValue = (original > depth) ? (original - depth) : 0;
  const uint8_t maxValue =
      static_cast<uint8_t>(std::min<int>(15, original + depth));
  const uint8_t span = static_cast<uint8_t>(maxValue - minValue + 1);
  const uint8_t randomized =
      static_cast<uint8_t>(minValue + ((GetRandomByte(system) * span) / 256));

  return static_cast<ushort>((originalParam & 0xFFF0) | randomized);
}

static ushort RandomizeLowByte(ushort originalParam, uint8_t amount,
                               uint8_t maximum, System *system) {
  return RandomizeValue(static_cast<uint8_t>(originalParam & 0xFF), amount,
                        maximum, system);
}

static ushort RandomizeLowBytePreservingUpper(ushort originalParam,
                                              uint8_t amount, uint8_t maximum,
                                              System *system) {
  return static_cast<ushort>(
      (originalParam & 0xFF00) |
      RandomizeLowByte(originalParam, amount, maximum, system));
}

static ushort RandomizeWholeSignedValue(ushort originalParam, ushort amount,
                                        System *system) {
  if (amount == 0x00) {
    return originalParam;
  }
  const int random = static_cast<int>(GetRandomByte(system));
  const int delta = ((random - 128) * static_cast<int>(amount)) / 127;
  const int original = static_cast<int>(static_cast<int16_t>(originalParam));
  return static_cast<ushort>(
      static_cast<int16_t>(std::clamp(original + delta, -32768, 32767)));
}

static ushort RandomizeWholeValue(ushort originalParam, ushort amount,
                                  ushort minimum, ushort maximum,
                                  System *system) {
  if (amount == 0x00) {
    return originalParam;
  }
  const int random = static_cast<int>(GetRandomByte(system));
  const int delta = ((random - 128) * static_cast<int>(amount)) / 127;
  return static_cast<ushort>(std::clamp(static_cast<int>(originalParam) + delta,
                                        static_cast<int>(minimum),
                                        static_cast<int>(maximum)));
}

static ushort RandomizeByteFields(ushort originalParam, uint8_t highAmount,
                                  uint8_t lowAmount, System *system) {
  const uint8_t high = RandomizeByte(
      static_cast<uint8_t>((originalParam >> 8) & 0xFF), highAmount, system);
  const uint8_t low = RandomizeByte(static_cast<uint8_t>(originalParam & 0xFF),
                                    lowAmount, system);
  return static_cast<ushort>((static_cast<ushort>(high) << 8) | low);
}

static ushort RandomizeCommandParam(TrackerCommand command,
                                    ushort originalParam, ushort amount,
                                    System *system) {
  if (amount == 0x00) {
    return originalParam;
  }

  const uint8_t highAmount = static_cast<uint8_t>((amount >> 8) & 0xFF);
  const uint8_t lowAmount = static_cast<uint8_t>(amount & 0xFF);

  ushort randomized = originalParam;
  switch (command) {
  case TrackerCommand::InstrumentCommandArpeggiator:
  case TrackerCommand::InstrumentCommandMidiChord:
    randomized = RandomizeNibbles(originalParam, amount, system);
    break;

  case TrackerCommand::InstrumentCommandCrush: {
    const uint8_t drive = RandomizeByte(
        static_cast<uint8_t>((originalParam >> 8) & 0xFF), highAmount, system);
    const uint8_t crush =
        RandomizeNibble(static_cast<uint8_t>(originalParam & 0x0F),
                        static_cast<uint8_t>(amount & 0x0F), system);
    randomized = static_cast<ushort>((static_cast<ushort>(drive) << 8) | crush);
    break;
  }

  case TrackerCommand::InstrumentCommandDelay:
    randomized = RandomizeLowNibble(
        originalParam, static_cast<uint8_t>(amount & 0x0F), system);
    break;

  case TrackerCommand::InstrumentCommandGateOff:
  case TrackerCommand::InstrumentCommandInstrumentRetrigger:
  case TrackerCommand::InstrumentCommandKill:
    randomized = RandomizeLowByte(originalParam, lowAmount, 0xFF, system);
    break;

  case TrackerCommand::InstrumentCommandVelocity:
    randomized = RandomizeLowByte(originalParam, lowAmount, 0x7F, system);
    break;

  case TrackerCommand::InstrumentCommandRetrigger:
    randomized =
        RandomizeByteFields(originalParam, highAmount, lowAmount, system);
    break;

  case TrackerCommand::InstrumentCommandGroove:
    randomized =
        RandomizeLowBytePreservingUpper(originalParam, lowAmount, 0xFF, system);
    break;

  case TrackerCommand::InstrumentCommandLoopOfset:
    randomized = RandomizeWholeSignedValue(originalParam, amount, system);
    break;

  case TrackerCommand::InstrumentCommandMidiCC:
    randomized = static_cast<ushort>(
        (originalParam & 0xFF00) |
        RandomizeLowByte(originalParam, lowAmount, 0x7F, system));
    break;

  case TrackerCommand::InstrumentCommandPlayOfset:
  case TrackerCommand::InstrumentCommandFilterCut:
  case TrackerCommand::InstrumentCommandLowPassFilter:
  case TrackerCommand::InstrumentCommandFilterResonance:
  case TrackerCommand::InstrumentCommandLegato:
  case TrackerCommand::InstrumentCommandPan:
  case TrackerCommand::InstrumentCommandPitchFineTune:
  case TrackerCommand::InstrumentCommandPitchSlide:
  case TrackerCommand::InstrumentCommandVolume:
    randomized =
        RandomizeByteFields(originalParam, highAmount, lowAmount, system);
    break;

  case TrackerCommand::InstrumentCommandTempo:
    randomized = RandomizeWholeValue(originalParam, amount, MIN_TEMPO,
                                     MAX_TEMPO, system);
    break;

  case TrackerCommand::InstrumentCommandHop: {
    uint8_t high = RandomizeByte(static_cast<uint8_t>(originalParam >> 8),
                                 highAmount, system);
    randomized = (static_cast<ushort>(high) << 8) | (originalParam & 0x00FF);
    randomized = RandomizeLowNibbleWithByteAmountPreservingUpper(
        randomized, lowAmount, system);
    break;
  }

  case TrackerCommand::InstrumentCommandChance:
  case TrackerCommand::InstrumentCommandMidiPC:
  case TrackerCommand::InstrumentCommandRandom:
  case TrackerCommand::InstrumentCommandStop:
  case TrackerCommand::InstrumentCommandTable:
  case TrackerCommand::InstrumentCommandNone:
  case TrackerCommand::Default:
    randomized = originalParam;
    break;
  }

  return CommandList::RangeLimitCommandParam(command, randomized);
}

static bool IsControlCommand(TrackerCommand command) {
  return command == TrackerCommand::InstrumentCommandChance ||
         command == TrackerCommand::InstrumentCommandRandom;
}

static uchar RandomizeNote(uchar note, uint8_t amount, System *system) {
  if (note > HIGHEST_NOTE) {
    return note;
  }
  return RandomizeValue(note, amount, HIGHEST_NOTE, system);
}

} // namespace

RowFxResult ResolveRowFx(uchar note, TrackerCommand cmd1, ushort param1,
                         TrackerCommand cmd2, ushort param2,
                         const PlayerLastActiveCommandState &,
                         bool includeCommandFx, bool enableNoteChance) {
  RowFxResult fx;
  fx.note = note;
  fx.cmd1 = cmd1;
  fx.param1 = param1;
  fx.cmd2 = cmd2;
  fx.param2 = param2;

  System *system = System::GetInstance();

  auto applyChance = [&](int column) {
    TrackerCommand cmd = (column == 1) ? fx.cmd1 : fx.cmd2;
    ushort param = (column == 1) ? fx.param1 : fx.param2;
    if (cmd != TrackerCommand::InstrumentCommandChance) {
      return;
    }

    const uint8_t amount = static_cast<uint8_t>(param & 0xFF);

    if (column == 1) {
      if (enableNoteChance) {
        fx.allowNote = fx.allowNote && RollChance(amount, system);
      }
      return;
    }

    if (fx.cmd1 != TrackerCommand::InstrumentCommandNone) {
      fx.allowCmd1 = fx.allowCmd1 && RollChance(amount, system);
    }
  };

  auto applyRandom = [&](int column) {
    TrackerCommand cmd = (column == 1) ? fx.cmd1 : fx.cmd2;
    ushort param = (column == 1) ? fx.param1 : fx.param2;
    bool allowed = (column == 1) ? fx.allowCmd1 : fx.allowCmd2;
    if (cmd != TrackerCommand::InstrumentCommandRandom) {
      return;
    }
    if (!allowed) {
      return;
    }

    const ushort amount = param;

    if (column == 1) {
      fx.note =
          RandomizeNote(fx.note, static_cast<uint8_t>(amount & 0xFF), system);
      return;
    }

    if (fx.cmd1 != TrackerCommand::InstrumentCommandNone) {
      fx.param1 = RandomizeCommandParam(fx.cmd1, fx.param1, amount, system);
    }
  };

  // Phase ordering: CHN first, then RND.
  applyChance(1);
  applyChance(2);

  if (includeCommandFx) {
    applyRandom(2);
  } else {
    applyRandom(1);
  }

  return fx;
}

int32_t ResolveRowHop(TrackerCommand cmd1, ushort param1, TrackerCommand cmd2,
                      ushort param2) {
  const PlayerLastActiveCommandState lastActive = {
      .lastCmd = TrackerCommand::InstrumentCommandNone,
      .lastParam = 0,
      .valid = false,
  };

  RowFxResult fx = ResolveRowFx(NO_NOTE, cmd1, param1, cmd2, param2, lastActive,
                                true, false);

  if (fx.allowCmd1 && fx.cmd1 == TrackerCommand::InstrumentCommandHop) {
    return static_cast<int32_t>(fx.param1 & 0x0F);
  }
  if (fx.allowCmd2 && fx.cmd2 == TrackerCommand::InstrumentCommandHop) {
    return static_cast<int32_t>(fx.param2 & 0x0F);
  }

  return -1;
}

TableRowFxResult ResolveTableRowFx(const TrackerCommand (&cmd)[TABLE_COLUMNS],
                                   const ushort (&param)[TABLE_COLUMNS],
                                   uchar currentNote,
                                   const PlayerLastActiveCommandState &,
                                   bool includeCommandFx) {
  TableRowFxResult fx;
  fx.note = currentNote;
  for (uint8_t column = 0; column < TABLE_COLUMNS; column++) {
    fx.cmd[column] = cmd[column];
    fx.param[column] = param[column];
  }

  System *system = System::GetInstance();

  auto applyChance = [&](uint8_t column) {
    if (fx.cmd[column] != TrackerCommand::InstrumentCommandChance) {
      return;
    }

    const uint8_t amount = static_cast<uint8_t>(fx.param[column] & 0xFF);

    if (column == 0) {
      fx.allowNote = fx.allowNote && RollChance(amount, system);
      return;
    }

    const uint8_t targetColumn = column - 1;
    if (fx.cmd[targetColumn] == TrackerCommand::InstrumentCommandNone) {
      return;
    }
    fx.allowCmd[targetColumn] =
        fx.allowCmd[targetColumn] && RollChance(amount, system);
  };

  auto applyRandom = [&](uint8_t column) {
    if (fx.cmd[column] != TrackerCommand::InstrumentCommandRandom) {
      return;
    }
    if (!fx.allowCmd[column]) {
      return;
    }

    const ushort amount = fx.param[column];

    if (column == 0) {
      if (fx.allowNote) {
        fx.note =
            RandomizeNote(fx.note, static_cast<uint8_t>(amount & 0xFF), system);
      }
      return;
    }

    const uint8_t targetColumn = column - 1;
    if (fx.cmd[targetColumn] == TrackerCommand::InstrumentCommandNone) {
      return;
    }
    fx.param[targetColumn] = RandomizeCommandParam(
        fx.cmd[targetColumn], fx.param[targetColumn], amount, system);
  };

  for (uint8_t column = 0; column < TABLE_COLUMNS; column++) {
    applyChance(column);
  }

  if (includeCommandFx) {
    for (uint8_t column = 0; column < TABLE_COLUMNS; column++) {
      applyRandom(column);
    }
  }

  return fx;
}

void UpdateLastActiveCommandState(PlayerLastActiveCommandState &state,
                                  TrackerCommand cmd, ushort param) {
  if (cmd == TrackerCommand::InstrumentCommandNone || IsControlCommand(cmd)) {
    return;
  }
  state.lastCmd = cmd;
  state.lastParam = param;
  state.valid = true;
}
