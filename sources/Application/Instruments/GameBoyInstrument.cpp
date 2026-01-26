/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 * Copyright (c) 2925 nILS Podewski
 *
 * This file is part of the picoTracker firmware
 */

#include "GameBoyInstrument.h"
#include "CommandList.h"
#include "Externals/etl/include/etl/to_string.h"
#include "I_Instrument.h"
#include "System/Profiler/Profiler.h"
#include "bit.h"
#include <string.h>

static const char *waveShapes[GB_NUM_WAVEFORMS] = {
    "Pulse 12.5%", "Pulse 25%", "Pulse 50%",     "Triangle",
    "Noise GB7",   "Noise NES", "Noise SN76489", "Noise White"};

voice_t GameBoyInstrument::voices_[SONG_CHANNEL_COUNT];

GameBoyInstrument::GameBoyInstrument()
    : I_Instrument(&variables_), vWaveform_(FourCC::GameBoyInstrumentWaveform,
                                            waveShapes, GB_NUM_WAVEFORMS, 0),
      vAttack_(FourCC::GameBoyInstrumentAttack, 0x00),
      vDecay_(FourCC::GameBoyInstrumentDecay, 0x80),
      vLevel_(FourCC::GameBoyInstrumentLevel, 0x80),
      vLength_(FourCC::GameBoyInstrumentLength, -1),
      vBurst_(FourCC::GameBoyInstrumentBurst, 0x00),
      vVibratoDepth_(FourCC::GameBoyInstrumentVibrato, 0x07),
      vVibratoDelay_(FourCC::GameBoyInstrumentVibratoDelay, 0x40),
      vTranspose_(FourCC::GameBoyInstrumentTranspose, 0x00),
      vTable_(FourCC::GameBoyInstrumentTable, 0x00),
      vArpSpeed_(FourCC::GameBoyInstrumentArpSpeed, 0x12),
      vSweepTime_(FourCC::GameBoyInstrumentSweepTime, 0x00),
      vSweepAmount_(FourCC::GameBoyInstrumentSweepAmount, 0x00) {

  // Initialize exported variables
  // name_ is now an etl::string in the base class, not a Variable
  variables_.insert(variables_.end(), &vWaveform_);
  variables_.insert(variables_.end(), &vAttack_);
  variables_.insert(variables_.end(), &vDecay_);
  variables_.insert(variables_.end(), &vLevel_);
  variables_.insert(variables_.end(), &vLength_);
  variables_.insert(variables_.end(), &vBurst_);
  variables_.insert(variables_.end(), &vVibratoDepth_);
  variables_.insert(variables_.end(), &vVibratoDelay_);
  variables_.insert(variables_.end(), &vTranspose_);
  variables_.insert(variables_.end(), &vTable_);
  variables_.insert(variables_.end(), &vArpSpeed_);
  variables_.insert(variables_.end(), &vSweepTime_);
  variables_.insert(variables_.end(), &vSweepAmount_);
}

void GameBoyInstrument::Stop(int channel) { voices_[channel].stop(); };

bool GameBoyInstrument::Start(int channel, unsigned char note, bool retrigger) {
  // note on get frequency etc.

  // dump instrument parameters
  InstrumentParameters params = getInstrumentParameters();
  voices_[channel].note_on(note, retrigger, params);

  return true;
};

bool GameBoyInstrument::Render(int channel, fixed *buffer, int size,
                               bool updateTick) {
  // PROFILE_SCOPE("GameBoyInstrument::Render");
  voice_t &v = voices_[channel];

  for (int s = 0; s < size; s++) {
    v.sample(buffer, buffer + 1);

    // Output to both channels
    buffer += 2;
  }

  return true;
};

void GameBoyInstrument::ProcessCommand(int channel, FourCC cc, ushort value) {
  switch (cc) {
  case FourCC::InstrumentCommandArpeggiator:
    voices_[channel].command_init_arp(value);
    break;

  case FourCC::InstrumentCommandKill:
  case FourCC::InstrumentCommandGateOff:
    voices_[channel].stop();
    break;

  case FourCC::InstrumentCommandVolume:
    voices_[channel].parameters.level = value;
    break;

  case FourCC::InstrumentCommandCrush:
    voices_[channel].bitcrush = value && 0x0f;
    voices_[channel].drive = value >> 8;
    break;

  case FourCC::InstrumentCommandPan:
    voices_[channel].command_init_pan(value >> 8, value & 0xFF);
    break;

  case FourCC::InstrumentCommandPitchSlide:
    break;

  case FourCC::InstrumentCommandLegato:
    voices_[channel].command_init_legato(value >> 8, (int8_t)(value & 0xFF));
    break;
  }
};

bool GameBoyInstrument::SupportsCommand(FourCC cc) { return false; }

InstrumentParameters GameBoyInstrument::getInstrumentParameters() {
  InstrumentParameters params;
  params.wave = vWaveform_.GetInt();
  params.attack = vAttack_.GetInt();
  params.decay = vDecay_.GetInt();
  params.level = vLevel_.GetInt();
  params.length = vLength_.GetInt();
  params.burst = vBurst_.GetInt();
  params.vibratoDepth = vVibratoDepth_.GetInt();
  params.vibratoDelay = vVibratoDelay_.GetInt();
  params.transpose = vTranspose_.GetInt();
  params.table = vTable_.GetInt();
  params.arpSpeed = vArpSpeed_.GetInt();
  params.sweepTime = vSweepTime_.GetInt();
  params.sweepAmount = vSweepAmount_.GetInt();
  return params;
}
