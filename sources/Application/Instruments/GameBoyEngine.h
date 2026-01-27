#pragma once

#include "Application/Utils/fixed.h"
#include "System/Console/Trace.h"
#include <cstdint>

static uint32_t lcg = 42;

constexpr int GB_NUM_WAVEFORMS = 8;
constexpr int SAMPLING_RATE = 44100;

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

/******************************************************************************
 * constants                                                                  *
 ******************************************************************************/

#include "CompileTimeFunctions.h"

// precalculated semitone ratios for pitch slides (Q16.16 format)
constexpr auto semitoneRatioQ16 = makeRatioLUT(std::make_index_sequence<256>{});
// precalculated frequency table midi notes -12 to 127+12
constexpr auto frequencyLUT = makeFreqLUT(std::make_index_sequence<128 + 24>{});
// precalculated attack coefficients for envelope (0-64)
constexpr auto attackCoeffLUT = makeAttackLUT(std::make_index_sequence<65>{});
// precalculated decay coefficients for envelope (0-64)
constexpr auto decayCoeffLUT = makeDecayLUT(std::make_index_sequence<65>{});
// precalculated sine wave values for vibrato (0-63 + sentinel)
constexpr auto sine64LUT = makeSine64LUT(std::make_index_sequence<65>{});

static inline uint32_t multiplyQ32(uint32_t a, uint32_t b) {
  uint64_t tmp = (uint64_t)a * b;
  return (uint32_t)(tmp >> 32);
}

static inline uint16_t interpolateU16(const uint16_t *lut, uint8_t v) {
  uint8_t idx = v >> 2; // 0..63
  uint8_t frac = v & 3; // 0..3

  uint16_t c0 = lut[idx];
  uint16_t c1 = lut[idx + 1]; // safe because of sentinel

  return c0 + (((uint32_t)(c1 - c0) * frac) >> 2);
}

static inline int8_t interpolateS8(const int8_t *lut, uint8_t v) {
  uint8_t idx = v >> 2; // 0..63
  uint8_t frac = v & 3; // 0..3

  int8_t c0 = lut[idx];
  int8_t c1 = lut[idx + 1]; // safe because of sentinel

  return c0 + (((int32_t)(c1 - c0) * frac) >> 2);
}

typedef struct InstrumentParameters {
  uint8_t wave;
  uint8_t attack;
  uint8_t decay;
  uint8_t level;
  int16_t length;
  uint8_t burst;
  uint8_t vibratoDepth;
  uint8_t vibratoDelay;
  int8_t transpose;
  int16_t table;
  uint8_t arpSpeed;
  uint8_t sweepTime;
  int8_t sweepAmount;
} InstrumentParameters;

typedef enum { ENV_IDLE = 0, ENV_ATTACK, ENV_DECAY } EnvState;

/******************************************************************************
 * envelope                                                                   *
 ******************************************************************************/

typedef struct {
  uint16_t value;
  uint16_t coefficient; // q0.16
  uint16_t attack;
  uint16_t decay;
  uint16_t target; // 0 or 65535
  EnvState state;

  void setAttack(uint8_t a) {
    attack = interpolateU16(attackCoeffLUT.data(), a);
  }

  void setDecay(uint8_t d) { decay = interpolateU16(decayCoeffLUT.data(), d); }

  void trigger() {
    value = 0;
    target = 65535;
    coefficient = attack;
    state = ENV_ATTACK;
  }

  void tick() {
    if (state == ENV_IDLE)
      return;

    uint32_t diff = (uint32_t)target - value;
    int32_t tmp = value + ((diff * coefficient) >> 16);

    if (state == ENV_ATTACK && tmp >= 65530) {
      tmp = 65535;
      target = 0;
      coefficient = decay;
      state = ENV_DECAY;
    } else if (tmp <= 10) {
      tmp = 0;
      state = ENV_IDLE;
    }

    value = tmp;
  }

} Envelope;

/******************************************************************************
 * voice                                                                      *
 ******************************************************************************/

typedef struct voice_t {
  InstrumentParameters parameters;

  bool arpeggio = false;
  uint16_t arpTick = 0;
  uint16_t arpTime = 250;
  uint8_t note;
  uint32_t phase = 0;
  int32_t frequency = 0;
  int32_t lastFrequency = 0;
  uint32_t arpLength = 5;
  int32_t arpFrequencies[5] = {0, 0, 0, 0, 0};
  uint8_t arpIndex = 0;
  uint32_t time;
  uint32_t tick;
  uint32_t tock;
  uint32_t lifetime;
  uint8_t wave;
  uint32_t volume;
  int32_t burstTime;
  uint16_t vibPhase;
  const uint16_t vibFrequency = 0xfff;
  int32_t vibDepth;
  int32_t vibSwing;
  uint32_t vibDelay;

  uint8_t drive;
  uint8_t bitcrush;

  int16_t pan;
  uint8_t panTarget;
  uint8_t panStep;

  FourCC command; // tODO remove

  uint32_t sweepCoefficient;
  int32_t sweepSteps;

  // legato (exponential pitch slide)
  int32_t legatoCoefficient = 0; // q16.16 multiplier per 100Hz tick
  int32_t legatoFactor = 0;      // q16.16 multiplier per 100Hz tick
  int32_t legatoSteps = 0;       // remaining ticks
  int32_t legatoTargetFreq = 0;  // target frequency

  bool legato = false;

  uint16_t lfsr = 17;
  uint32_t noise;

  uint32_t lastSample = 0;
  const int32_t maxStep = 0x3fff'ffff;
  const int32_t minStep = -0x3fff'ffff;

  Envelope envelope;

  inline void stop() {
    frequency = 0;
    phase = 0;
  }

  void runCommand() {
    switch (command) {
    case FourCC::InstrumentCommandArpeggiator: {
      // handled in 1000Hz tick
      break;
    }
    default:
      break;
    }
  }

  inline void sample(fixed *left, fixed *right) {
    uint32_t combinedGain = (volume * envelope.value) >> 16; // precompute

    uint8_t leftGain = std::min((0xff - pan) * 2, 0xff);
    uint8_t rightGain = std::min(0xff, 2 * pan);

    // cold loop @ 100 Hz ------------------------------------------------------
    if (tick == 0) {

      // envelope processing at ~100Hz
      tick = 441;
      envelope.tick();

      // recompute combined gain when envelope changes
      combinedGain = (volume * envelope.value) >> 16;

      // handle commands
      runCommand();

      // sweep
      if (sweepSteps) {
        sweepSteps--;

        // sweep al base frequencies
        for (uint32_t i = 0; i < arpLength; i++) {
          uint64_t f = arpFrequencies[i];
          f *= sweepCoefficient;
          arpFrequencies[i] = uint32_t(f >> 16);
        }
      }

      // pan

      if (panStep) {
        if (pan < panTarget) {
          pan += panStep;
          if (pan > panTarget) {
            pan = panTarget;
            panStep = 0;
          }
        } else if (pan > panTarget) {
          pan -= panStep;
          if (pan < panTarget) {
            pan = panTarget;
            panStep = 0;
          }
        }

        uint8_t leftGain = std::min((0xff - pan) * 2, 0xff);
        uint8_t rightGain = std::min(0xff, 2 * pan);
      }

      // vibrato
      int delta = 0;

      if (time > vibDelay) {
        vibPhase += vibFrequency;
        int32_t sine = interpolateS8(sine64LUT.data(), vibPhase >> 8);
        delta = (vibSwing * sine) >> 7;
        delta = (vibDepth * delta) >> 8;
      }

      frequency = arpFrequencies[arpIndex] + delta;

      // legato: exponential frequency interpolation (Q16.16 fixed-point)
      if (legatoSteps > 0 && legato) {
        // accumulate factor: legatoFactor *= legatoCoefficient (both Q16.16)
        legatoFactor += legatoCoefficient;
        legatoSteps--;

        if (legatoSteps == 0) {
          legatoFactor = legatoTargetFreq;
        }
      }

      // apply to frequency: frequency *= legatoFactor (Q16.16)
      frequency = ((int64_t)frequency * legatoFactor) >> 16;
    }

    // warm loop @ ~1000 Hz ----------------------------------------------------
    if (tock == 0) {
      // hot loop

      // envelope processing at ~1000Hz
      tock = 44;

      // arpeggio
      if (arpeggio) {
        arpTick++;

        if (arpTick >= arpTime) {
          arpTick = 0;
          arpIndex++;
          if (arpIndex >= arpLength) {
            arpIndex = 0;
          };
        }
      }

      if (lifetime == 0) {
        // note off
        wave = gbWaveNone;
        envelope.state = ENV_IDLE;
      } else {
        if (burstTime > 0) {
          burstTime--;
          wave = gbWaveNoiseWhite;
        } else {
          wave = parameters.wave;
        }

        // length
        lifetime--;
      }
    }

    // hot loop @ ~44100 Hz ----------------------------------------------------
    tick--;
    tock--;

    // advance phase
    phase += frequency;

    fixed sample = 0;

    // generate sample based on waveform
    switch (wave) {
    case gbWavePulse12_5: // pulse 12.5%
      sample = pulse(phase > 0x2000'0000);
      break;
    case gbWavePulse25: // pulse 25%
      sample = pulse(phase > 0x4000'0000);
      break;
    case gbWavePulse50: // pulse 50%
      sample = pulse(phase > 0x8000'0000);
      break;
    case gbWaveTriangle: // triangle
      if (phase < 0x8000'0000) {
        // first half, rising slope
        sample = phase >> 3;
      } else {
        // second half, falling slope
        sample = (0xFFFF'FFFF - phase) >> 3;
      }
      sample &= 0xFF00'0000; // downsample
      break;
    case gbWaveNoiseGameBoy: // noise: GB7
    case gbWaveNoiseNES:     // noise: NES
    case gbWaveNoiseSN76489: // noise: SN76489
      if (phase > 0x4000'0000) {
        phase -= 0x4000'0000;
        noise = (wave == gbWaveNoiseGameBoy) ? voice_noise_gb7(&lfsr)
                : (wave == gbWaveNoiseNES)   ? voice_noise_nes(&lfsr)
                                             : voice_noise_sn76489(&lfsr);
      }
      sample = noise;
      break;
    case gbWaveNoiseWhite: // noise: white noise, frequency independent
      lcg *= 1664525;
      lcg += 1013904223;
      sample = lcg & 0x0FFF'FFFF;
      break;
    }

    time++;

    // apply combined gain (volume * envelope) in single operation
    sample = (sample >> 8) * combinedGain;

    // apply bitcrush
    if (bitcrush) {
      sample &= (0xffff'ffff << bitcrush);
      // drive is ignored at the moment
      // sample *= drive;
    }

    // apply panning

    *left = (sample >> 8) * leftGain;
    *right = (sample >> 8) * rightGain;
  }

  inline void note_on(unsigned char note, bool retrigger,
                      InstrumentParameters parameters) {
    this->parameters = parameters;

    // is this the best time to store that?
    lastFrequency = frequency;

    // command settings
    legatoSteps = 0;
    legatoFactor = 0x0001'0000; // 1.0 in Q16.16
    legato = false;
    arpeggio = false;

    bitcrush = 0;
    drive = 0;

    pan = 128;
    panTarget = 128;
    panStep = 0;

    int fIndex = std::clamp(note + 12 + parameters.transpose, 0, 127 + 24);
    frequency = frequencyLUT[fIndex];
    arpFrequencies[0] = frequency;

    this->note = note;

    command = FourCC::InstrumentCommandNone;

    arpTime = 35 - parameters.arpSpeed;
    arpIndex = 0;
    arpTick = 0;
    phase = 0;
    time = 0;
    tick = 0;
    tock = 0;
    legatoCoefficient = 0x0001'0000; // 1.0 in Q16.16

    // reset vibrato
    vibSwing = frequencyLUT[fIndex + 1] - frequency;
    vibDelay = parameters.vibratoDelay << 8;
    vibDepth = parameters.vibratoDepth;
    vibPhase = 0;

    // reset envelope
    envelope.setAttack(parameters.attack);
    envelope.setDecay(parameters.decay);
    envelope.trigger();

    lifetime = (parameters.length < 0) ? 0x7FFF'FFFF : (1 + parameters.length);

    wave = parameters.wave;
    volume = parameters.level;
    burstTime = parameters.burst;

    // sweep
    int32_t sweepDepth = parameters.sweepAmount;
    sweepCoefficient = (1 << 16) + (sweepDepth * 64);
    sweepSteps = parameters.sweepTime;
  }

  /* waveform generation ******************************************************/

  inline uint32_t pulse(bool level) {
    uint32_t target = -(uint32_t)level & 0x0FFF'FFFF;

    int32_t diff = (int32_t)target - (int32_t)lastSample;

    if (diff > maxStep) {
      diff = maxStep;
    } else if (diff < minStep) {
      diff = minStep;
    }

    return (lastSample = (lastSample + diff));
  }

  static inline uint32_t voice_noise_lfsr(uint16_t *lfsr, int b, int feedback) {
    uint16_t lfsr_val = *lfsr;

    bool bitA = lfsr_val & 1;
    bool bitB = (lfsr_val >> b) & 1;
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

  /* command implementation ***************************************************/

  // integer log2 approximation for Q16.16 input
  static inline int32_t log2_fixed(uint32_t xQ16) {
    if (xQ16 == 0)
      return 0;

    // integer part of log2
    int leading = 31 - __builtin_clz(xQ16);
    // normalized fractional part
    uint32_t frac = xQ16 << (31 - leading);
    // take top 16 bits as fraction
    int32_t frac16 = frac >> 15;
    return (leading - 16) << 16 | (frac16 & 0xFFFF);
  }

  // integer exp2 approximation, input in Q16.16, output Q16.16
  static inline uint32_t exp2_fixed(int32_t log2Q16) {
    int32_t intPart = log2Q16 >> 16;
    int32_t fracPart = log2Q16 & 0xFFFF;
    // 2^fracPart ~ 1 + fracPart / 65536 (linear approx)
    uint32_t result = (1U << 16) + fracPart;
    // multiply by 2^intPart
    if (intPart >= 0)
      result <<= intPart;
    else
      result >>= -intPart;
    return result;
  }

  void command_init_pan(uint8_t speed, int8_t pan) {
    panTarget = pan;
    panStep = speed;
  }

  // fully fixed-point per-tick legato initialization
  void command_init_legato(uint8_t speed, int8_t semitones) {
    // not exponential in the GameBoy instrument, for performance reasons (atm)
    int ticks = 1 + speed; // minimum 1 tick

    if (semitones == 0) {
      // slide from lastFrequency to current frequency
      if (lastFrequency == frequency) {
        // same frequency - no slide needed
        legato = false;
        return;
      }

      // initial factor = lastFrequency / frequency (e.g., 0.5 for up an octave)
      // target factor = 1.0 (when we reach the new frequency)
      int64_t initialFactor = ((int64_t)lastFrequency << 16) / frequency;
      legatoTargetFreq = 0x0001'0000; // 1.0 in Q16.16
      legatoCoefficient = (legatoTargetFreq - initialFactor) / ticks;
      legatoFactor = initialFactor; // start at the ratio
    } else {
      // clamp semitones to table
      if (semitones < -128)
        semitones = -128;
      if (semitones > 127)
        semitones = 127;

      // get total ratio from table (Q16.16)
      legatoTargetFreq = semitoneRatioQ16[semitones + 128];
      legatoCoefficient = (legatoTargetFreq - 0x0001'0000) / ticks;
    }

    legatoSteps = ticks;
    legatoFactor = 0x0001'0000; // start at 1.0

    legato = true;
  }

  void command_init_arp(ushort value) {
    arpIndex = 0;
    arpLength = 5;

    arpeggio = true;

    // trim off trailing zeroes
    uint16_t val = value;

    while (arpLength > 1 && (val & 0xF) == 0) {
      arpLength--;
      val >>= 4;
    }

    for (uint8_t i = 0; i < arpLength - 1; i++) {
      uint8_t semitone = (value >> (i * 4)) & 0x0F;
      int32_t noteVal = note + parameters.transpose + semitone + 12;
      noteVal = (noteVal < 0) ? 0 : noteVal;
      noteVal = (noteVal > 127 + 24) ? 127 + 24 : noteVal;
      arpFrequencies[i + 1] = frequencyLUT[noteVal];
    }
  }
} voice_t;