/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "CommandList.h"

static FourCC _all[] = {
    FourCC::InstrumentCommandNone,
    FourCC::InstrumentCommandArpeggiator,
    FourCC::InstrumentCommandCrush,
    FourCC::InstrumentCommandDelay,
    FourCC::InstrumentCommandFilterCut,
    FourCC::InstrumentCommandLowPassFilter,
    FourCC::InstrumentCommandFilterResonance,
    FourCC::InstrumentCommandGateOff,
    FourCC::InstrumentCommandGroove,
    FourCC::InstrumentCommandHop,
    FourCC::InstrumentCommandRetrigger,
    FourCC::InstrumentCommandInstrumentRetrigger,
    FourCC::InstrumentCommandKill,
    FourCC::InstrumentCommandLegato,
    FourCC::InstrumentCommandLoopOfset,
    FourCC::InstrumentCommandMidiCC,
    FourCC::InstrumentCommandMidiChord,
    FourCC::InstrumentCommandMidiPC,
    FourCC::InstrumentCommandPan,
    FourCC::InstrumentCommandPitchFineTune,
    FourCC::InstrumentCommandPlayOfset,
    FourCC::InstrumentCommandPitchSlide,
    FourCC::InstrumentCommandStop,
    FourCC::InstrumentCommandTable,
    FourCC::InstrumentCommandTempo,
    FourCC::InstrumentCommandVelocity,
    FourCC::InstrumentCommandVibrato,
    FourCC::InstrumentCommandVolume,
};

// Applies command-specific range limits to parameter values
ushort CommandList::RangeLimitCommandParam(FourCC command, ushort paramValue) {
  // Each command type can have its own specific range limits
  if (command == FourCC::InstrumentCommandVelocity) {
    // For VEL command, limit the bb part to 0x7F (127) while preserving the aa
    // part
    return (paramValue & 0xFF00) | (paramValue & 0x7F);
  }
  // Add more command-specific limits here as needed
  // Example:
  // else if (command == FourCC::InstrumentCommandMidiCC) {
  //   // MIDI CC values should also be limited to 0-127
  //   return (paramValue & 0xFF00) | (paramValue & 0x7F);
  // }

  // If no specific limit applies, return the original value
  return paramValue;
}

FourCC CommandList::GetNext(FourCC current) {
  for (uint i = 0; i < sizeof(_all) / sizeof(FourCC) - 1; i++) {
    if (_all[i] == current) {
      return _all[i + 1];
    };
  };
  return current;
};

FourCC CommandList::GetPrev(FourCC current) {
  uint count = sizeof(_all) / sizeof(FourCC);
  for (uint i = 2; i < count; i++) {
    if (_all[i] == current) {
      return _all[i - 1];
    };
  };
  return current;
};

FourCC CommandList::GetNextAlpha(FourCC current) {
  char letter = ((char *)&current)[0];
  bool found = false;
  for (uint i = 0; i < sizeof(_all) / sizeof(FourCC); i++) {
    char tLetter = ((char *)&_all[i])[0];
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

FourCC CommandList::GetPrevAlpha(FourCC current) {

  char letter = ((char *)&current)[0];
  bool found = false;
  FourCC tReturn = FourCC::Default;
  uint count = sizeof(_all) / sizeof(FourCC);

  for (uint i = count - 1; i > 0; i--) {
    char tLetter = ((char *)&_all[i])[0];
    if (!found) {
      if (tLetter == letter) {
        found = true;
      }
    } else {
      if (tLetter != letter) {
        if (tReturn == 0xFF) {
          tReturn = _all[i];
        } else {
          if (tLetter != ((char *)&tReturn)[0]) {
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
