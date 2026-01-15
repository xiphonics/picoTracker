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

inline uint32_t GameBoyInstrument::pulse(bool level) {
  uint32_t target = -(uint32_t)level & 0x0FFF'FFFF;

  int32_t diff = (int32_t)target - (int32_t)lastSample_;

  if (diff > maxStep_) {
    diff = maxStep_;
  } else if (diff < minStep_) {
    diff = minStep_;
  }

  return (lastSample_ = (lastSample_ + diff));
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
    : I_Instrument(&variables_), vWaveform_(FourCC::GameBoyInstrumentWaveform,
                                            waveShapes, GB_NUM_WAVEFORMS, 0),
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

  envelope_.setAttack(vAttack_.GetInt());
  envelope_.setDecay(vDecay_.GetInt());
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

  int fIndex = std::clamp(note + 12 + vTranspose_.GetInt(), 0, 127 + 24);
  frequency_ = frequencyTable[fIndex];
  arpFrequencies_[0] = frequency_;

  note_ = note;

  command_ = FourCC::InstrumentCommandNone;

  arpTime_ = 35 - vArpSpeed_.GetInt();
  arpIndex_ = 0;
  arpTick_ = 0;
  phase_ = 0;
  time_ = 0;
  tick_ = 0;
  tock_ = 0;

  // reset vibrato
  vibSwing_ = frequencyTable[fIndex + 1] - frequency_;
  vibDelay_ = vVibratoDelay_.GetInt() << 8;
  vibDepth_ = vVibratoDepth_.GetInt();
  vibPhase_ = 0;

  // reset envelope
  envelope_.setAttack(vAttack_.GetInt());
  envelope_.setDecay(vDecay_.GetInt());
  envelope_.trigger();

  int v = vLength_.GetInt();
  lifetime_ = v ? v : 0xFFFF'FFFF;

  wave_ = vWaveform_.GetInt();
  volume_ = vLevel_.GetInt();
  burstTime_ = vBurst_.GetInt();

  // sweep
  int32_t sweepDepth = vSweepAmount_.GetInt();
  sweepCoefficient_ = (1 << 16) + (sweepDepth * 64);
  sweepSteps_ = vSweepTime_.GetInt();

  return true;
};

void GameBoyInstrument::Stop(int c) {
  frequency_ = 0;
  phase_ = 0;
};

bool GameBoyInstrument::Render(int channel, fixed *buffer, int size,
                               bool updateTick) {
  PROFILE_SCOPE("GameBoyInstrument::Render");
  uint32_t wave = wave_;

  for (int s = 0; s < size; s++) {
    // cold loop @ 100 Hz ------------------------------------------------------
    if (tick_ == 0) {

      // envelope processing at ~100Hz
      tick_ = 441;

      envelope_.tick();

      // handle commands
      RunCommand();

      // sweep
      if (sweepSteps_) {
        sweepSteps_--;

        // sweep al base frequencies
        for (uint32_t i = 0; i < arpLength_; i++) {
          uint64_t f = arpFrequencies_[i];
          f *= sweepCoefficient_;
          arpFrequencies_[i] = uint32_t(f >> 16);
        }
      }

      // vibrato
      int delta = 0;

      if (time_ > vibDelay_) {
        vibPhase_ += vibFrequency_;
        int32_t sine = interpolateS8(sine64, (vibPhase_ >> 8));
        delta = (vibSwing_ * sine) >> 7;
        delta = (vibDepth_ * delta) >> 8;
      }

      frequency_ = arpFrequencies_[arpIndex_] + delta;
    }

    // warm loop @ ~1000 Hz ----------------------------------------------------
    if (tock_ == 0) {
      // hot loop

      // envelope processing at ~1000Hz
      tock_ = 44;

      // arpeggio
      if (command_ == FourCC::InstrumentCommandArpeggiator) {
        arpTick_++;

        if (arpTick_ >= arpTime_) {
          arpTick_ = 0;
          arpIndex_++;
          if (arpIndex_ >= arpLength_) {
            arpIndex_ = 0;
          };
        }
      }

      // length
      lifetime_--;

      if (lifetime_ == 0) {
        // note off
        wave_ = gbWaveNone;
        envelope_.state = ENV_IDLE;
      }

      if (burstTime_) {
        burstTime_--;
        wave = gbWaveNoiseWhite;
      } else {
        wave = wave_;
      }
    }

    // hot loop @ ~44100 Hz ----------------------------------------------------
    tick_--;
    tock_--;

    // advance phase
    phase_ += frequency_;

    fixed sample = 0;

    // generate sample based on waveform
    switch (wave) {
    case gbWavePulse12_5: // Pulse 12.5%
      sample = pulse(phase_ > 0x2000'0000);
      break;
    case gbWavePulse25: // Pulse 25%
      sample = pulse(phase_ > 0x4000'0000);
      break;
    case gbWavePulse50: // Pulse 50%
      sample = pulse(phase_ > 0x8000'0000);
      break;
    case gbWaveTriangle: // Triangle
      if (phase_ < 0x8000'0000) {
        // first half, rising slope
        sample = phase_ >> 3;
      } else {
        // second half, falling slope
        sample = (0xFFFF'FFFF - phase_) >> 3;
      }
      sample &= 0xFF00'0000; // downsample
      break;
    case gbWaveNoiseGameBoy: // Noise: GB7
      if (phase_ > 0x4000'0000) {
        phase_ -= 0x4000'0000;
        noise_ = voice_noise_gb7(&lfsr_);
        sample = noise_;
      } else {
        sample = noise_;
      }
      break;
    case gbWaveNoiseNES: // Noise: NES
      if (phase_ > 0x4000'0000) {
        phase_ -= 0x4000'0000;
        noise_ = voice_noise_nes(&lfsr_);
        sample = noise_;
      } else {
        sample = noise_;
      }
      break;
    case gbWaveNoiseSN76489: // Noise: SN76489
      if (phase_ > 0x4000'0000) {
        phase_ -= 0x4000'0000;
        noise_ = voice_noise_sn76489(&lfsr_);
        sample = noise_;
      } else {
        sample = noise_;
      }
      break;
    case gbWaveNoiseWhite: // Noise: White Noise, frequency independent
      sample = rand() & 0x0FFF'FFFF;
      break;
    }

    // apply master level
    sample >>= 8;
    sample *= volume_;

    // apply envelope level 32 * 8 * 16 = 56 bits >> 24 to get back to 32
    sample >>= 16;
    sample *= envelope_.value; // 32

    // output to both left and right channels
    buffer[s * 2] = sample;
    buffer[s * 2 + 1] = sample;

    time_++;
  }

  return true;
};

bool GameBoyInstrument::IsInitialized() {
  return true; // Always initialised
};

void GameBoyInstrument::RunCommand() {
  switch (command_) {
  case FourCC::InstrumentCommandArpeggiator: {
    // handled in 1000Hz tick
    break;
  }
  default:
    break;
  }
}

void GameBoyInstrument::CommandInitArp(int channel, ushort value) {
  arpIndex_ = 0;
  arpLength_ = 5;

  // trim off trailing zeroes
  ushort v = value;
  while (arpLength_ > 1 && (v & 0xF) == 0) {
    arpLength_--;
    v >>= 4;
  }

  for (uint32_t i = 0; i < arpLength_ - 1; i++) {
    uint8_t semitone = (value >> (i * 4)) & 0x0F;
    int32_t value = note_ + vTranspose_.GetInt() + semitone + 12;
    value = (value < 0) ? 0 : value;
    value = (value > 127 + 24) ? 127 + 24 : value;
    arpFrequencies_[i + 1] = frequencyTable[value];
  }
  Trace::Error("Initialized an arp command: %d steps", arpLength_);
}

void GameBoyInstrument::ProcessCommand(int channel, FourCC cc, ushort value) {
  switch (cc) {
  case FourCC::InstrumentCommandArpeggiator:
    command_ = cc;
    CommandInitArp(channel, value);
    break;

  case FourCC::InstrumentCommandKill:
  case FourCC::InstrumentCommandGateOff:
    command_ = cc;
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
