/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include <cstdio>
#include <cstring>

// CAUTION: all strings must fit in the line length limits!
// First line is max 31 - MAX_BATTERY_GAUGE_WIDTH, second line is max 31
// chars
static char **getHelpLegend(FourCC command) {
  static char *result[2];
  result[1] = (char *)("bb at speed aa");
  switch (command) {
  case FourCC::InstrumentCommandKill:
    result[0] = (char *)("KILl:--bb, stop playing");
    result[1] = (char *)("after bb ticks");
    break;
  case FourCC::InstrumentCommandLoopOfset:
    result[0] = (char *)("Loop OFset: Shift loop");
    result[1] = (char *)("start & end values aaaa");
    break;
  case FourCC::InstrumentCommandArpeggiator:
    result[0] = (char *)("ARPeggio: Cycle through");
    result[1] = (char *)("relative pitches");
    break;
  case FourCC::InstrumentCommandVolume:
    result[0] = (char *)("VOLume:aabb, reach volume");
    break;
  case FourCC::InstrumentCommandVelocity:
    result[0] = (char *)("VELocity:--bb,");
    result[1] = (char *)("MIDI velocity");
    break;
  case FourCC::InstrumentCommandPitchSlide:
    result[0] = (char *)("Pitch SLide:aabb, to pitch");
    break;
  case FourCC::InstrumentCommandHop:
    result[0] = (char *)("Hop:aabb, hop to bb");
    result[1] = (char *)("aa times");
    break;
  case FourCC::InstrumentCommandLegato:
    result[0] = (char *)("Legato:aabb, slide");
    result[1] = (char *)("to pitch bb at speed aa");
    break;
  case FourCC::InstrumentCommandRetrigger:
    result[0] = (char *)("Retrigger:aabb retrigger");
    result[1] = (char *)("loop bb ticks at speed aa");
    break;
  case FourCC::InstrumentCommandTempo:
    result[0] = (char *)("Tempo:--bb, set tempo to");
    result[1] = (char *)("hex value bb");
    break;
  case FourCC::InstrumentCommandMidiCC:
    result[0] = (char *)("MIDI CC:aabb, ");
    result[1] = (char *)("CC message aa value bb");
    break;
  case FourCC::InstrumentCommandMidiPC:
    result[0] = (char *)("MIDI Program Change:aabb,");
    result[1] = (char *)("send program change bb");
    break;
  case FourCC::InstrumentCommandPlayOfset:
    result[0] = (char *)("Play OFfset:aabb, jump abs");
    result[1] = (char *)("to aa or move rel bb chunks");
    break;
  case FourCC::InstrumentCommandFilterResonance:
    result[0] = (char *)("FiLTer&Res:aabb, cutoff aa");
    result[1] = (char *)("resonance bb");
    break;
  case FourCC::InstrumentCommandLowPassFilter:
    result[0] = (char *)("FiLTeR:aabb, resonance to");
    break;
  case FourCC::InstrumentCommandTable:
    result[0] = (char *)("TaBLe:--bb, run table bb");
    break;
  case FourCC::InstrumentCommandCrush:
    result[0] = (char *)("drive&CruSH:aa-b,");
    result[1] = (char *)("drive aa crush -b");
    break;
  case FourCC::InstrumentCommandFilterCut:
    result[0] = (char *)("FilterCuToff:aabb, cutoff");
    break;
  case FourCC::InstrumentCommandPan:
    result[0] = (char *)("PAN:aabb, pan to value");
    break;
  case FourCC::InstrumentCommandGroove:
    result[0] = (char *)("GRooVe:aabb, set groove bb");
    result[1] = (char *)("if aa > 0, set all tracks");
    break;
  case FourCC::InstrumentCommandInstrumentRetrigger:
    result[0] = (char *)("InstrumentReTrig: --bb");
    result[1] = (char *)("retrigger & transpose by bb");
    break;
  case FourCC::InstrumentCommandPitchFineTune:
    result[0] = (char *)("PitchFineTune:aabb, tune");
    break;
  case FourCC::InstrumentCommandDelay:
    result[0] = (char *)("Delay:--bb, delay bb tics");
    break;
  case FourCC::InstrumentCommandStop:
    result[0] = (char *)("Stop playing song now");
    break;
  case FourCC::InstrumentCommandGateOff:
    result[0] = (char *)("GateOff (Synth only)");
    break;
  case FourCC::InstrumentCommandMidiChord:
    result[0] = (char *)("MIDI Chord:abcd");
    result[1] = (char *)("send rel notes:+a,+b,+c,+d");
    break;
  default:
    result[0] = result[1] = (char *)("");
    break;
  }
  return result;
}
