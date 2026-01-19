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

// precalculated frequency table midi notes -12 to 127+12
static const int32_t frequencyTable[128 + 24] = {
    398127,     421801,     446882,     473455,     501608,     531436,
    563036,     596516,     631987,     669567,     709381,     751563,
    796254,     843601,     893765,     946911,     1003217,    1062871,
    1126073,    1193033,    1263974,    1339134,    1418763,    1503127,
    1592507,    1687203,    1787529,    1893821,    2006434,    2125742,
    2252146,    2386065,    2527948,    2678268,    2837526,    3006254,
    3185015,    3374406,    3575058,    3787642,    4012867,    4251485,
    4504291,    4772130,    5055896,    5356535,    5675051,    6012507,
    6370030,    6748811,    7150117,    7575285,    8025735,    8502970,
    9008582,    9544261,    10111792,   10713070,   11350103,   12025015,
    12740059,   13497623,   14300233,   15150569,   16051469,   17005939,
    18017165,   19088521,   20223584,   21426141,   22700205,   24050030,
    25480119,   26995246,   28600467,   30301139,   32102938,   34011878,
    36034330,   38177043,   40447168,   42852281,   45400411,   48100060,
    50960238,   53990491,   57200933,   60602278,   64205876,   68023757,
    72068660,   76354085,   80894335,   85704563,   90800821,   96200119,
    101920476,  107980983,  114401866,  121204555,  128411753,  136047513,
    144137319,  152708170,  161788671,  171409126,  181601643,  192400238,
    203840952,  215961966,  228803732,  242409110,  256823506,  272095026,
    288274639,  305416341,  323577341,  342818251,  363203285,  384800477,
    407681904,  431923931,  457607465,  484818220,  513647012,  544190053,
    576549277,  610832681,  647154683,  685636503,  726406571,  769600953,
    815363807,  863847862,  915214929,  969636441,  1027294024, 1088380105,
    1153098554, 1221665363, 1294309365, 1371273005, 1452813141, 1539201906,
    1630727614, 1727695724, 1830429858, 1939272882, 2054588048, 2116760211,
    2116197109, 2113330725};

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

  voice_t &v = voices_[channel];
  int fIndex = std::clamp(note + 12 + vTranspose_.GetInt(), 0, 127 + 24);
  v.frequency = frequencyTable[fIndex];
  v.arpFrequencies[0] = v.frequency;

  v.note = note;

  v.command = FourCC::InstrumentCommandNone;

  v.arpTime = 35 - vArpSpeed_.GetInt();
  v.arpIndex = 0;
  v.arpTick = 0;
  v.phase = 0;
  v.time = 0;
  v.tick = 0;
  v.tock = 0;

  // reset vibrato
  v.vibSwing = frequencyTable[fIndex + 1] - v.frequency;
  v.vibDelay = vVibratoDelay_.GetInt() << 8;
  v.vibDepth = vVibratoDepth_.GetInt();
  v.vibPhase = 0;

  // reset envelope
  v.envelope.setAttack(vAttack_.GetInt());
  v.envelope.setDecay(vDecay_.GetInt());
  v.envelope.trigger();

  int len = vLength_.GetInt();
  v.lifetime = len ? len : 0xFFFF'FFFF;

  v.wave = vWaveform_.GetInt();
  v.volume = vLevel_.GetInt();
  v.burstTime = vBurst_.GetInt();

  // sweep
  int32_t sweepDepth = vSweepAmount_.GetInt();
  v.sweepCoefficient = (1 << 16) + (sweepDepth * 64);
  v.sweepSteps = vSweepTime_.GetInt();

  return true;
};

void GameBoyInstrument::Stop(int channel) {
  voices_[channel].frequency = 0;
  voices_[channel].phase = 0;
};

bool GameBoyInstrument::Render(int channel, fixed *buffer, int size,
                               bool updateTick) {
  PROFILE_SCOPE("GameBoyInstrument::Render");
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
