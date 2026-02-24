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
  result[1] = (char *)("                               ");
  switch (command) {
  case FourCC::InstrumentCommandKill:
    result[0] = (char *)("KILl: --bb");
    result[1] = (char *)("stop playing after bb ticks");
    break;
  case FourCC::InstrumentCommandLoopOfset:
    result[0] = (char *)("Loop OFset: aaaa");
    result[1] = (char *)("Shift loop start & end values aaaa");
    break;
  case FourCC::InstrumentCommandArpeggiator:
    result[0] = (char *)("ARPeggio: abcd");
    result[1] = (char *)("Cycle thru relative pitch abcd");
    break;
  case FourCC::InstrumentCommandVolume:
    result[0] = (char *)("VOLume:aabb");
    result[1] = (char *)("reach volume bb at speed aa");
    break;
  case FourCC::InstrumentCommandVelocity:
    result[0] = (char *)("VELocity:--bb");
    result[1] = (char *)("send MIDI velocity cmd bb");
    break;
  case FourCC::InstrumentCommandPitchSlide:
    result[0] = (char *)("Pitch SLide: aabb");
    result[1] = (char *)("speed aa, slide to pitch bb");
    break;
  case FourCC::InstrumentCommandHop:
    result[0] = (char *)("Hop: aabb");
    result[1] = (char *)("hop to bb aa times");
    break;
  case FourCC::InstrumentCommandLegato:
    result[0] = (char *)("Legato: aabb");
    result[1] = (char *)("slide to pitch bb at speed aa");
    break;
  case FourCC::InstrumentCommandRetrigger:
    result[0] = (char *)("Retrigger: aabb");
    result[1] = (char *)("SAMPL:bb loop+aa ofst, MIDI:bb");
    break;
  case FourCC::InstrumentCommandTempo:
    result[0] = (char *)("Tempo: aabb");
    result[1] = (char *)("set tempo to hex value aabb");
    break;
  case FourCC::InstrumentCommandMidiCC:
    result[0] = (char *)("MIDI CC: aabb");
    result[1] = (char *)("CC message aa value bb");
    break;
  case FourCC::InstrumentCommandMidiPC:
    result[0] = (char *)("MIDI Program Change: --bb");
    result[1] = (char *)("send program change bb");
    break;
  case FourCC::InstrumentCommandPlayOfset:
    result[0] = (char *)("Play OFfset: aabb");
    result[1] = (char *)("jmp abs aa & mv rel signed bb");
    break;
  case FourCC::InstrumentCommandFilterResonance:
    result[0] = (char *)("FiLTer&Res: aabb");
    result[1] = (char *)("speed aa, resonance bb");
    break;
  case FourCC::InstrumentCommandLowPassFilter:
    result[0] = (char *)("FiLTeR: aabb");
    result[1] = (char *)("cutoff aa, resonance bb");
    break;
  case FourCC::InstrumentCommandTable:
    result[0] = (char *)("TaBLe: --bb");
    result[1] = (char *)("run table bb");
    break;
  case FourCC::InstrumentCommandCrush:
    result[0] = (char *)("drive&CruSH: aa-b");
    result[1] = (char *)("drive aa crush -b");
    break;
  case FourCC::InstrumentCommandFilterCut:
    result[0] = (char *)("FilterCuToff: aabb");
    result[1] = (char *)("speed aa, target cutoff bb");
    break;
  case FourCC::InstrumentCommandPan:
    result[0] = (char *)("PAN: aabb");
    result[1] = (char *)("speed aa, value bb (00 right)");
    break;
  case FourCC::InstrumentCommandGroove:
    result[0] = (char *)("GRooVe:aabb");
    result[1] = (char *)("set bb (aa > 0,set all tracks)");
    break;
  case FourCC::InstrumentCommandInstrumentRetrigger:
    result[0] = (char *)("InstrumentReTrig: --bb");
    result[1] = (char *)("retrigger & transpose by bb");
    break;
  case FourCC::InstrumentCommandPitchFineTune:
    result[0] = (char *)("PitchFineTune:aabb");
    result[1] = (char *)("speed aa, tune bb (~+/-1 st)");
    break;
  case FourCC::InstrumentCommandDelay:
    result[0] = (char *)("Delay:---b");
    result[1] = (char *)("delay b+1 ticks");
    break;
  case FourCC::InstrumentCommandStop:
    result[0] = (char *)("Stop table playback");
    result[1] = (char *)("");
    break;
  case FourCC::InstrumentCommandGateOff:
    result[0] = (char *)("GateOff (Synth only)");
    result[1] = (char *)("");
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
