/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 nILS Podewski
 *
 * This file is part of the picoTracker firmware
 */

#include "ChiptuneInstrument.h"
#include "Foundation/Constants/SpecialCharacters.h"
#include "I_Instrument.h"
#include <string.h>

static const char *waveShapes[numWaveforms] = {
    char_waveform_pulse_s " 12.5%",   char_waveform_pulse_s " 25%",
    char_waveform_pulse_s " 50%",     char_waveform_tri_s "4bit",
    char_waveform_noise_s " GB7",     char_waveform_noise_s " NES",
    char_waveform_noise_s " SN76489", char_waveform_noise_s " White"};

voice_t ChiptuneInstrument::voices_[SONG_CHANNEL_COUNT];

ChiptuneInstrument::ChiptuneInstrument()
    : I_Instrument(&variables_),
      vArpSpeed_(FourCC::ChiptuneInstrumentArpSpeed, defaultArpSpeed),
      vAttack_(FourCC::ChiptuneInstrumentAttack, defaultAttack),
      vBurst_(FourCC::ChiptuneInstrumentBurst, defaultBurst),
      vDecay_(FourCC::ChiptuneInstrumentDecay, defaultDecay),
      vLength_(FourCC::ChiptuneInstrumentLength, defaultLength),
      vLevel_(FourCC::ChiptuneInstrumentLevel, defaultLevel),
      vSweepAmount_(FourCC::ChiptuneInstrumentSweepAmount, defaultSweepAmount),
      vSweepTime_(FourCC::ChiptuneInstrumentSweepTime, defaultSweepTime),
      vTable_(FourCC::ChiptuneInstrumentTable, defaultTable),
      vTranspose_(FourCC::ChiptuneInstrumentTranspose, defaultTranspose),
      vVibratoDelay_(FourCC::ChiptuneInstrumentVibratoDelay,
                     defaultVibratoDelay),
      vVibratoDepth_(FourCC::ChiptuneInstrumentVibrato, defaultVibratoDepth),
      vWaveform_(FourCC::ChiptuneInstrumentWaveform, waveShapes, numWaveforms,
                 defaultWaveform) {
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

<<<<<<< HEAD
  params.wave = (chiptuneWaveType)vWaveform_.GetInt();
=======
  params.wave = (chiptune_wave_type_e)vWaveform_.GetInt();
>>>>>>> d81ac5f3 (refactoring and cleanup)
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
