/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "CommandList.h"

// Keep command entries grouped by displayed mnemonic first letter;
// GetNextAlpha/GetPrevAlpha depend on this ordering.
static TrackerCommand _all[] = {
    TrackerCommand::InstrumentCommandNone,
    TrackerCommand::InstrumentCommandArpeggiator,
    TrackerCommand::InstrumentCommandCrush,
    TrackerCommand::InstrumentCommandDelay,
    TrackerCommand::InstrumentCommandFilterCut,
    TrackerCommand::InstrumentCommandLowPassFilter,
    TrackerCommand::InstrumentCommandFilterResonance,
    TrackerCommand::InstrumentCommandGateOff,
    TrackerCommand::InstrumentCommandGroove,
    TrackerCommand::InstrumentCommandHop,
    TrackerCommand::InstrumentCommandInstrumentRetrigger,
    TrackerCommand::InstrumentCommandKill,
    TrackerCommand::InstrumentCommandLegato,
    TrackerCommand::InstrumentCommandLoopOfset,
    TrackerCommand::InstrumentCommandMidiCC,
    TrackerCommand::InstrumentCommandMidiChord,
    TrackerCommand::InstrumentCommandMidiPC,
    TrackerCommand::InstrumentCommandPan,
    TrackerCommand::InstrumentCommandPitchFineTune,
    TrackerCommand::InstrumentCommandPlayOfset,
    TrackerCommand::InstrumentCommandPitchSlide,
    TrackerCommand::InstrumentCommandRetrigger,
    TrackerCommand::InstrumentCommandStop,
    TrackerCommand::InstrumentCommandTable,
    TrackerCommand::InstrumentCommandTempo,
    TrackerCommand::InstrumentCommandVelocity,
    TrackerCommand::InstrumentCommandVolume,
};

static char GetCommandGroupLetter(TrackerCommand command) {
  const char *name = TrackerCommand(command).c_str();
  return (name && name[0]) ? name[0] : '\0';
}

// Applies command-specific range limits to parameter values
ushort CommandList::RangeLimitCommandParam(TrackerCommand command,
                                           ushort paramValue) {
  // Each command type can have its own specific range limits
  if (command == TrackerCommand::InstrumentCommandVelocity) {
    // For VEL command, limit the bb part to 0x7F (127) while preserving the aa
    // part
    return (paramValue & 0xFF00) | (paramValue & 0x7F);
  }
  // Add more command-specific limits here as needed
  // Example:
  // else if (command == TrackerCommand::InstrumentCommandMidiCC) {
  //   // MIDI CC values should also be limited to 0-127
  //   return (paramValue & 0xFF00) | (paramValue & 0x7F);
  // }

  // If no specific limit applies, return the original value
  return paramValue;
}

TrackerCommand CommandList::GetNext(TrackerCommand current) {
  for (uint i = 0; i < sizeof(_all) / sizeof(TrackerCommand) - 1; i++) {
    if (_all[i] == current) {
      return _all[i + 1];
    };
  };
  return current;
};

TrackerCommand CommandList::GetPrev(TrackerCommand current) {
  uint count = sizeof(_all) / sizeof(TrackerCommand);
  for (uint i = 2; i < count; i++) {
    if (_all[i] == current) {
      return _all[i - 1];
    };
  };
  return current;
};

TrackerCommand CommandList::GetNextAlpha(TrackerCommand current) {
  char letter = GetCommandGroupLetter(current);
  bool found = false;
  for (uint i = 0; i < sizeof(_all) / sizeof(TrackerCommand); i++) {
    char tLetter = GetCommandGroupLetter(_all[i]);
    if (!found) {
      if (tLetter == letter) {
        found = true;
      }
    } else {
      if (tLetter != letter) {
        return _all[i];
      }
    };
  };
  return current;
};

TrackerCommand CommandList::GetPrevAlpha(TrackerCommand current) {

  char letter = GetCommandGroupLetter(current);
  bool found = false;
  TrackerCommand tReturn = TrackerCommand::Default;
  uint count = sizeof(_all) / sizeof(TrackerCommand);

  for (uint i = count - 1; i > 0; i--) {
    char tLetter = GetCommandGroupLetter(_all[i]);
    if (!found) {
      if (tLetter == letter) {
        found = true;
      }
    } else {
      if (tLetter != letter) {
        if (tReturn == 0xFF) {
          tReturn = _all[i];
        } else {
          if (tLetter != GetCommandGroupLetter(tReturn)) {
            return tReturn;
          } else {
            tReturn = _all[i];
          }
        }
      }
    };
  };
  if (tReturn != 0xFF) {
    return tReturn;
  }
  return current;
};
