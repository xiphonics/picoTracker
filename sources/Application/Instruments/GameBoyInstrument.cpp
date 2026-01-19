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
#include "System/Console/Trace.h"
#include "System/Profiler/Profiler.h"
#include "bit.h"
#include <string.h>

enum gbWaveType {
  gbWavePulse12_5,
  gbWavePulse25,
  gbWavePulse50,
  gbWaveTriangle,
  gbWaveNoiseGameBoy,
  gbWaveNoiseNES,
  gbWaveNoiseSN76489,
  gbWaveNoiseWhite,
  gbWaveNone
};

static const char *waveShapes[GB_NUM_WAVEFORMS] = {
    "Pulse 12.5%", "Pulse 25%", "Pulse 50%",     "Triangle",
    "Noise GB7",   "Noise NES", "Noise SN76489", "Noise White"};

#define CHANNEL 0 // just hardcoding to channel 0 for now

static uint32_t lcg = 42;
voice_t GameBoyInstrument::voices_[SONG_CHANNEL_COUNT];

inline uint32_t GameBoyInstrument::pulse(int channel, bool level) {
  uint32_t target = -(uint32_t)level & 0x0FFF'FFFF;

  int32_t diff = (int32_t)target - (int32_t)voices_[channel].lastSample;

  if (diff > voices_[channel].maxStep) {
    diff = voices_[channel].maxStep;
  } else if (diff < voices_[channel].minStep) {
    diff = voices_[channel].minStep;
  }

  return (voices_[channel].lastSample = (voices_[channel].lastSample + diff));
}

static inline uint32_t voice_noise_lfsr(uint16_t *lfsr, int b1, int feedback) {
  uint16_t lfsr_val = *lfsr;

  bool bitA = lfsr_val & 1;
  bool bitB = (lfsr_val >> b1) & 1;
  bool bitF = bitA ^ bitB;

  lfsr_val = (lfsr_val >> 1) | (bitF << feedback);

  *lfsr = lfsr_val;
  return bitA ? 0x0FFF'FFFF : 0;
}

static inline uint32_t voice_noise_nes(uint16_t *lfsr) {
  return voice_noise_lfsr(lfsr, 6, 14);
}

static inline uint32_t voice_noise_gb7(uint16_t *lfsr) {
  return voice_noise_lfsr(lfsr, 1, 6);
}

static inline uint32_t voice_noise_sn76489(uint16_t *lfsr) {
  return voice_noise_lfsr(lfsr, 3, 14);
}

GameBoyInstrument::GameBoyInstrument()
    : I_Instrument(&variables_),
      vWaveform_(FourCC::GameBoyInstrumentWaveform, waveShapes, GB_NUM_WAVEFORMS, 0),
      vAttack_(FourCC::GameBoyInstrumentAttack, 0x00),
      vDecay_(FourCC::GameBoyInstrumentDecay, 0x80),
      vLevel_(FourCC::GameBoyInstrumentLevel, 0x80),
      vLength_(FourCC::GameBoyInstrumentLength, 0x00),
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

GameBoyInstrument::~GameBoyInstrument(){};

bool GameBoyInstrument::Init() {
  // enable left/right only for 0 channel

  return true;
};

void GameBoyInstrument::OnStart(){};

bool GameBoyInstrument::Start(int channel, unsigned char note, bool retrigger) {
  // note on get frequency etc.

  Trace::Error("GameBoyInstrument: Start note %d on channel %d", note, channel);

  // dump instrument parameters
  voice_t &v = voices_[channel];
  v.parameters = getInstrumentParameters();
  v.note_on(note, retrigger);

  return true;
};

void GameBoyInstrument::Stop(int channel) {
  voices_[channel].frequency = 0;
  voices_[channel].phase = 0;
};

bool GameBoyInstrument::Render(int channel, fixed *buffer, int size,
                               bool updateTick) {
  // PROFILE_SCOPE("GameBoyInstrument::Render");
  voice_t &v = voices_[channel];
  uint32_t wave = v.wave;
  uint32_t combinedGain = (v.volume * v.envelope.value) >> 16; // Precompute

  for (int s = 0; s < size; s++) {
    // cold loop @ 100 Hz ------------------------------------------------------
    if (v.tick == 0) {

      // envelope processing at ~100Hz
      v.tick = 441;
      v.envelope.tick();

      // handle commands
      RunCommand(channel);

      // sweep
      if (v.sweepSteps) {
        v.sweepSteps--;

        // sweep al base frequencies
        for (uint32_t i = 0; i < v.arpLength; i++) {
          uint64_t f = v.arpFrequencies[i];
          f *= v.sweepCoefficient;
          v.arpFrequencies[i] = uint32_t(f >> 16);
        }
      }

      // vibrato
      int delta = 0;

      if (v.time > v.vibDelay) {
        v.vibPhase += v.vibFrequency;
        int32_t sine = interpolateS8(sine64, (v.vibPhase >> 8));
        delta = (v.vibSwing * sine) >> 7;
        delta = (v.vibDepth * delta) >> 8;
      }

      v.frequency = v.arpFrequencies[v.arpIndex] + delta;
    }

    // warm loop @ ~1000 Hz ----------------------------------------------------
    if (v.tock == 0) {
      // hot loop

      // envelope processing at ~1000Hz
      v.tock = 44;

      // arpeggio
      if (v.command == FourCC::InstrumentCommandArpeggiator) {
        v.arpTick++;

        if (v.arpTick >= v.arpTime) {
          v.arpTick = 0;
          v.arpIndex++;
          if (v.arpIndex >= v.arpLength) {
            v.arpIndex = 0;
          };
        }
      }

      // length
      v.lifetime--;

      if (v.lifetime == 0) {
        // note off
        v.wave = gbWaveNone;
        v.envelope.state = ENV_IDLE;
      }

      if (v.burstTime) {
        v.burstTime--;
        wave = gbWaveNoiseWhite;
      } else {
        wave = v.wave;
      }
      
      // Recompute combined gain when envelope or wave changes
      combinedGain = (v.volume * v.envelope.value) >> 16;
    }

    // hot loop @ ~44100 Hz ----------------------------------------------------
    v.tick--;
    v.tock--;

    // advance phase
    v.phase += v.frequency;

    fixed sample = 0;

    // generate sample based on waveform
    switch (wave) {
    case gbWavePulse12_5: // Pulse 12.5%
      sample = pulse(channel, v.phase > 0x2000'0000);
      break;
    case gbWavePulse25: // Pulse 25%
      sample = pulse(channel, v.phase > 0x4000'0000);
      break;
    case gbWavePulse50: // Pulse 50%
      sample = pulse(channel, v.phase > 0x8000'0000);
      break;
    case gbWaveTriangle: // Triangle
      if (v.phase < 0x8000'0000) {
        // first half, rising slope
        sample = v.phase >> 3;
      } else {
        // second half, falling slope
        sample = (0xFFFF'FFFF - v.phase) >> 3;
      }
      sample &= 0xFF00'0000; // downsample
      break;
    case gbWaveNoiseGameBoy: // Noise: GB7
      if (v.phase > 0x4000'0000) {
        v.phase -= 0x4000'0000;
        v.noise = voice_noise_gb7(&v.lfsr);
      }
      sample = v.noise;
      break;
    case gbWaveNoiseNES: // Noise: NES
      if (v.phase > 0x4000'0000) {
        v.phase -= 0x4000'0000;
        v.noise = voice_noise_nes(&v.lfsr);
      }
      sample = v.noise;
      break;
    case gbWaveNoiseSN76489: // Noise: SN76489
      if (v.phase > 0x4000'0000) {
        v.phase -= 0x4000'0000;
        v.noise = voice_noise_sn76489(&v.lfsr);
      }
      sample = v.noise;
      break;
    case gbWaveNoiseWhite: // Noise: White Noise, frequency independent
        lcg *= 1664525;
        lcg += 1013904223;
        sample = lcg & 0x0FFF'FFFF;
      break;
    }

    // Apply combined gain (volume * envelope) in single operation
    sample = (sample >> 8) * combinedGain;

    // Output to both channels
    buffer[0] = buffer[1] = sample;
    buffer += 2;
    
    v.time++;
  }

  return true;
};

bool GameBoyInstrument::IsInitialized() {
  return true; // Always initialised
};

void GameBoyInstrument::RunCommand(int channel) {
  switch (voices_[channel].command) {
  case FourCC::InstrumentCommandArpeggiator: {
    // handled in 1000Hz tick
    break;
  }
  default:
    break;
  }
}

void GameBoyInstrument::CommandInitArp(int channel, ushort value) {
  voice_t &v = voices_[channel];
  v.arpIndex = 0;
  v.arpLength = 5;

  // trim off trailing zeroes
  ushort val = value;
  while (v.arpLength > 1 && (val & 0xF) == 0) {
    v.arpLength--;
    val >>= 4;
  }

  for (uint32_t i = 0; i < v.arpLength - 1; i++) {
    uint8_t semitone = (value >> (i * 4)) & 0x0F;
    int32_t noteVal = v.note + vTranspose_.GetInt() + semitone + 12;
    noteVal = (noteVal < 0) ? 0 : noteVal;
    noteVal = (noteVal > 127 + 24) ? 127 + 24 : noteVal;
    v.arpFrequencies[i + 1] = frequencyTable[noteVal];
  }
  Trace::Error("Initialized an arp command: %d steps", v.arpLength);
}

void GameBoyInstrument::ProcessCommand(int channel, FourCC cc, ushort value) {
  switch (cc) {
  case FourCC::InstrumentCommandArpeggiator:
    voices_[channel].command = cc;
    CommandInitArp(channel, value);
    break;

  case FourCC::InstrumentCommandKill:
  case FourCC::InstrumentCommandGateOff:
    voices_[channel].command = cc;
    Stop(channel);
    break;
  }
};

int GameBoyInstrument::GetTable() { return vTable_.GetInt(); };

bool GameBoyInstrument::GetTableAutomation() {
  // Variable *v = FindVariable(MIP_TABLEAUTO);
  // return v->GetBool();
  return 0;
};

void GameBoyInstrument::GetTableState(TableSaveState &state) {}

void GameBoyInstrument::SetTableState(TableSaveState &state) {}

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
