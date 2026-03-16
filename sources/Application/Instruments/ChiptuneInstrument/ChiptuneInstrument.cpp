/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 * Copyright (c) 2925 nILS Podewski
 *
 * This file is part of the picoTracker firmware
 */

#include "ChiptuneInstrument.h"
#include "I_Instrument.h"
#include <string.h>

static const char *waveShapes[chiptuneNumWaveforms] = {
    "Pulse 12.5%", "Pulse 25%", "Pulse 50%",     "Triangle",
    "Noise GB7",   "Noise NES", "Noise SN76489", "Noise White"};

voice_t ChiptuneInstrument::voices_[SONG_CHANNEL_COUNT];

ChiptuneInstrument::ChiptuneInstrument()
    : I_Instrument(&variables_),
      vArpSpeed_(FourCC::ChiptuneInstrumentArpSpeed, 0x12),
      vAttack_(FourCC::ChiptuneInstrumentAttack, 0x00),
      vBurst_(FourCC::ChiptuneInstrumentBurst, -1),
      vDecay_(FourCC::ChiptuneInstrumentDecay, 0x80),
      vLength_(FourCC::ChiptuneInstrumentLength, -1),
      vLevel_(FourCC::ChiptuneInstrumentLevel, 0x80),
      vSweepAmount_(FourCC::ChiptuneInstrumentSweepAmount, 0x00),
      vSweepTime_(FourCC::ChiptuneInstrumentSweepTime, 0x00),
      vTable_(FourCC::ChiptuneInstrumentTable, -1),
      vTranspose_(FourCC::ChiptuneInstrumentTranspose, 0),
      vVibratoDelay_(FourCC::ChiptuneInstrumentVibratoDelay, 0x40),
      vVibratoDepth_(FourCC::ChiptuneInstrumentVibrato, 0x07),
      vWaveform_(FourCC::ChiptuneInstrumentWaveform, waveShapes,
                 chiptuneNumWaveforms, 0) {
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

void ChiptuneInstrument::Stop(int channel) { voices_[channel].stop(); };

bool ChiptuneInstrument::Start(int channel, unsigned char note,
                               bool retrigger) {
  // get the instrument parameters from the instrument and pass them to the
  // current voice
  voices_[channel].note_on(note, retrigger, getInstrumentParameters());

  return true;
}

bool ChiptuneInstrument::Render(int channel, fixed *buffer, int size,
                                bool updateTick) {
  // PROFILE_SCOPE("ChiptuneInstrument::Render");
  voice_t &v = voices_[channel];

  for (int s = 0; s < size; s++) {
    v.sample(buffer, buffer + 1);

    // Output to both channels
    buffer += 2;
  }

  return true;
}

void ChiptuneInstrument::ProcessCommand(int channel, FourCC cc, ushort value) {
  switch (cc) {
  case FourCC::InstrumentCommandArpeggiator:
    voices_[channel].command_init_arp(value);
    break;

  case FourCC::InstrumentCommandKill:
  case FourCC::InstrumentCommandGateOff:
    voices_[channel].stop();
    break;

  case FourCC::InstrumentCommandCrush:
    voices_[channel].bitcrush = value && 0x0f;
    voices_[channel].drive = value >> 8;
    break;

  case FourCC::InstrumentCommandVibrato:
    voices_[channel].command_init_vibrato(value >> 8, value & 0xFF);
    break;

  case FourCC::InstrumentCommandPan:
    voices_[channel].command_init_pan(value >> 8, value & 0xFF);
    break;

  case FourCC::InstrumentCommandPitchSlide:
    voices_[channel].command_init_pitch_shift(value >> 8, value & 0xFF);
    break;

  case FourCC::InstrumentCommandLegato:
    voices_[channel].command_init_legato(value >> 8, (int8_t)(value & 0xFF));
    break;

  case FourCC::InstrumentCommandVolume:
    voices_[channel].command_init_volume(value >> 8, value & 0xFF);
    break;

  case FourCC::InstrumentCommandPitchFineTune:
    voices_[channel].command_init_finetune(value >> 8, (int8_t)(value & 0xFF));
    break;

  case FourCC::InstrumentCommandInstrumentRetrigger:
    voices_[channel].command_init_instrument_retrigger(value >> 8,
                                                       (int8_t)(value & 0xff));
    break;
  }
}

bool ChiptuneInstrument::SupportsCommand(FourCC cc) { return false; }

InstrumentParameters ChiptuneInstrument::getInstrumentParameters() {
  InstrumentParameters params;

  params.wave = (chiptuneWaveType)vWaveform_.GetInt();
  params.attack = vAttack_.GetInt();
  params.decay = vDecay_.GetInt();
  params.level = vLevel_.GetInt();
  // off == -1, map to uint8_t range
  params.length = vLength_.GetInt() < 0 ? 0 : vLength_.GetInt();
  params.burst = vBurst_.GetInt() < 0 ? 0 : vBurst_.GetInt();
  params.vibratoDepth = vVibratoDepth_.GetInt();
  params.vibratoDelay = vVibratoDelay_.GetInt();
  params.transpose = vTranspose_.GetInt();
  params.arpSpeed = vArpSpeed_.GetInt();
  params.sweepTime = vSweepTime_.GetInt();
  params.sweepAmount = vSweepAmount_.GetInt();

  return params;
}
