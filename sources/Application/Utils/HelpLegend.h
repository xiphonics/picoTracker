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

  result[1] = (char *)"";

  switch (command) {
  case FourCC::InstrumentCommandKill:
    result[0] = (char *)"KILl: --bb, stop playing";
    result[1] = (char *)"after bb ticks";
    break;
  case FourCC::InstrumentCommandLoopOfset:
    result[0] = (char *)"Loop OFset: aaaa shift";
    result[1] = (char *)"loop start & end by aaaa";
    break;
  case FourCC::InstrumentCommandArpeggiator:
    result[0] = (char *)"ARPeggio: abcd, cycle";
    result[1] = (char *)"rel. pitches +a,+b,+c,+d";
    break;
  case FourCC::InstrumentCommandVolume:
    result[0] = (char *)"VOLume: aabb, approach";
    result[1] = (char *)"volume bb at speed aa";
    break;
  case FourCC::InstrumentCommandVelocity:
    result[0] = (char *)"VELocity: --bb, send MIDI";
    result[1] = (char *)"velocity bb";
    break;
  case FourCC::InstrumentCommandPitchSlide:
    result[0] = (char *)"Pitch SLide: aabb, slide";
    result[1] = (char *)"to pitch bb at speed aa";
    break;
  case FourCC::InstrumentCommandHop:
    result[0] = (char *)"HOP: aabb, hop to bb";
    result[1] = (char *)"aa times";
    break;
  case FourCC::InstrumentCommandLegato:
    result[0] = (char *)"LEGato: aabb, slide";
    result[1] = (char *)"to pitch bb at speed aa";
    break;
  case FourCC::InstrumentCommandRetrigger:
    result[0] = (char *)"ReTriGger: aabb retrigger";
    result[1] = (char *)"loop bb ticks at speed aa";
    break;
  case FourCC::InstrumentCommandTempo:
    result[0] = (char *)"TemPO: --bb, set tempo to";
    result[1] = (char *)"hex value bb";
    break;
  case FourCC::InstrumentCommandMidiCC:
    result[0] = (char *)"Midi CC: aabb, send";
    result[1] = (char *)"CC message aa value bb";
    break;
  case FourCC::InstrumentCommandMidiPC:
    result[0] = (char *)"Midi Program Change: --bb";
    result[1] = (char *)"send program change bb";
    break;
  case FourCC::InstrumentCommandPlayOfset:
    result[0] = (char *)"Play OFfset: aabb, go abs";
    result[1] = (char *)"to aa or move rel bb chunks";
    break;
  case FourCC::InstrumentCommandFilterResonance:
    result[0] = (char *)"Filter ReS: aabb, set";
    result[1] = (char *)"resonance bb at speed aa";
    break;
  case FourCC::InstrumentCommandLowPassFilter:
    result[0] = (char *)"FiLTer: aabb, set cutoff";
    result[1] = (char *)"aa & resonance bb";
    break;
  case FourCC::InstrumentCommandTable:
    result[0] = (char *)"TaBLe: --bb, run table bb";
    break;
  case FourCC::InstrumentCommandCrush:
    result[0] = (char *)"drive & CruSH: aa-b, set";
    result[1] = (char *)"drive aa & crush b";
    break;
  case FourCC::InstrumentCommandFilterCut:
    result[0] = (char *)"FilterCuToff: aabb, set";
    result[1] = (char *)"cutoff bb at speed aa";
    break;
  case FourCC::InstrumentCommandPan:
    result[0] = (char *)"PAN: aabb, pan to value,";
    result[1] = (char *)"00 is left, FF is right";
    break;
  case FourCC::InstrumentCommandGroove:
    result[0] = (char *)"GRooVe: aabb, set groove";
    result[1] = (char *)"bb, if aa>0 set all tracks";
    break;
  case FourCC::InstrumentCommandInstrumentRetrigger:
    result[0] = (char *)"InstrumentReTrig: aabb,";
    result[1] = (char *)"trig & transp. aa speed bb";
    break;
  case FourCC::InstrumentCommandPitchFineTune:
    result[0] = (char *)"PitchFineTune: aabb, tune";
    result[1] = (char *)"offset bb at speed bb";
    break;
  case FourCC::InstrumentCommandDelay:
    result[0] = (char *)"DeLaY: --bb, delay by bb";
    result[1] = (char *)"tics";
    break;
  case FourCC::InstrumentCommandStop:
    result[0] = (char *)"SToP: Stop playing the";
    result[1] = (char *)"song now";
    break;
  case FourCC::InstrumentCommandGateOff:
    result[0] = (char *)"Gate OFf: Set the gate";
    result[1] = (char *)"off for synths only";
    break;
  case FourCC::InstrumentCommandMidiChord:
    result[0] = (char *)"Midi CHord: abcd";
    result[1] = (char *)"Send rel notes: +a,+b,+c,+d";
    break;
  default:
    result[0] = result[1] = (char *)"";
    break;
  }
  return result;
}
