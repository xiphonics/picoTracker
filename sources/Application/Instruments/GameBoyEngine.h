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

// Precalculated semitone ratios for pitch slides (Q16.16 format)
// Index 128 = 1.0 (0 semitones), index 0 = 2^(-128/12), index 255 = 2^(127/12)
constexpr auto semitoneRatioQ16 =
    makeSemitoneRatioTable(std::make_index_sequence<256>{});

// precalculated frequency table midi notes -12 to 127+12
constexpr int32_t frequencyTable[128 + 24] = {
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

constexpr uint16_t attackCoeffLUT[65] = {
    65535, 63000, 60000, 55329, 45156, 35045, 27227, 21474, 17253, 14090, 11690,
    9844,  8384,  7221,  6279,  5508,  4869,  4334,  3881,  3495,  3163,  2876,
    2627,  2408,  2215,  2045,  1892,  1757,  1636,  1526,  1428,  1338,  1256,
    1182,  1114,  1052,  995,   943,   893,   849,   807,   769,   732,   699,
    668,   638,   612,   586,   562,   539,   517,   498,   479,   461,   444,
    428,   413,   399,   385,   372,   360,   348,   337,   326,   326,
};

constexpr uint16_t decayCoeffLUT[65] = {
    65535, 60000, 54134, 37479, 25165, 17618, 12848, 9728, 7602, 6089, 4980,
    4148,  3505,  3000,  2594,  2267,  1997,  1773,  1584, 1424, 1286, 1168,
    1065,  975,   896,   826,   764,   709,   659,   616,  575,  538,  506,
    476,   448,   423,   400,   378,   359,   340,   323,  308,  293,  280,
    267,   255,   244,   234,   224,   215,   206,   198,  191,  184,  177,
    170,   164,   159,   153,   148,   143,   138,   134,  130,  130,
};

constexpr const int8_t sine64[65] = {
    0,    12,   25,   37,   49,   60,   71,   81,   90,   98,   106,
    112,  117,  122,  125,  126,  127,  126,  125,  122,  117,  112,
    106,  98,   90,   81,   71,   60,   49,   37,   25,   12,   0,
    -12,  -25,  -37,  -49,  -60,  -71,  -81,  -90,  -98,  -106, -112,
    -117, -122, -125, -126, -127, -126, -125, -122, -117, -112, -106,
    -98,  -90,  -81,  -71,  -60,  -49,  -37,  -25,  -12,  0};

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
  int wave;
  int attack;
  int decay;
  int level;
  int length;
  int burst;
  int vibratoDepth;
  int vibratoDelay;
  int transpose;
  int table;
  int arpSpeed;
  int sweepTime;
  int sweepAmount;
} InstrumentParameters;

typedef enum { ENV_IDLE = 0, ENV_ATTACK, ENV_DECAY } EnvState;

/******************************************************************************
 * envelope                                                                   *
 ******************************************************************************/

typedef struct {
  uint16_t value;       // 0..65535
  uint16_t coefficient; // Q0.16
  uint16_t attackCoeff;
  uint16_t decayCoeff;
  uint16_t target; // 0 or 65535
  EnvState state;

  void setAttack(uint8_t a) { attackCoeff = interpolateU16(attackCoeffLUT, a); }
  void setDecay(uint8_t d) { decayCoeff = interpolateU16(decayCoeffLUT, d); }

  void trigger() {
    value = 0;
    target = 65535;
    coefficient = attackCoeff;
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
      coefficient = decayCoeff;
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
  uint32_t egState;
  uint32_t egLevel;
  uint32_t egAttackRate;
  uint32_t egDecayRate;
  uint32_t time;
  uint32_t tick;
  uint32_t tock;
  uint32_t lifetime;
  uint32_t wave;
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

  FourCC command; // TODO remove

  uint32_t sweepCoefficient;
  int32_t sweepSteps;

  // Legato (exponential pitch slide)
  int32_t legatoCoefficient = 0; // Q16.16 multiplier per 100Hz tick
  int32_t legatoFactor = 0;      // Q16.16 multiplier per 100Hz tick
  int32_t legatoSteps = 0;       // Remaining ticks
  int32_t legatoTargetFreq = 0;  // Target frequency

  bool legato = false;

  uint16_t lfsr = 17;
  uint32_t noise;

  uint32_t lastSample = 0;
  int32_t maxStep = 0x3fff'ffff;
  int32_t minStep = -0x3fff'ffff;

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
    uint32_t combinedGain = (volume * envelope.value) >> 16; // Precompute
    uint32_t waveform = wave;

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
        int32_t sine = interpolateS8(sine64, (vibPhase >> 8));
        delta = (vibSwing * sine) >> 7;
        delta = (vibDepth * delta) >> 8;
      }

      frequency = arpFrequencies[arpIndex] + delta;

      // Legato: exponential frequency interpolation (Q16.16 fixed-point)
      if (legatoSteps > 0 && legato) {
        // Accumulate factor: legatoFactor *= legatoCoefficient (both Q16.16)
        legatoFactor += legatoCoefficient;
        legatoSteps--;

        if (legatoSteps == 0) {
          legatoFactor = legatoTargetFreq;
        }
      }

      // Apply to frequency: frequency *= legatoFactor (Q16.16)
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
    case gbWavePulse12_5: // Pulse 12.5%
      sample = pulse(phase > 0x2000'0000);
      break;
    case gbWavePulse25: // Pulse 25%
      sample = pulse(phase > 0x4000'0000);
      break;
    case gbWavePulse50: // Pulse 50%
      sample = pulse(phase > 0x8000'0000);
      break;
    case gbWaveTriangle: // Triangle
      if (phase < 0x8000'0000) {
        // first half, rising slope
        sample = phase >> 3;
      } else {
        // second half, falling slope
        sample = (0xFFFF'FFFF - phase) >> 3;
      }
      sample &= 0xFF00'0000; // downsample
      break;
    case gbWaveNoiseGameBoy: // Noise: GB7
      if (phase > 0x4000'0000) {
        phase -= 0x4000'0000;
        noise = voice_noise_gb7(&lfsr);
      }
      sample = noise;
      break;
    case gbWaveNoiseNES: // Noise: NES
      if (phase > 0x4000'0000) {
        phase -= 0x4000'0000;
        noise = voice_noise_nes(&lfsr);
      }
      sample = noise;
      break;
    case gbWaveNoiseSN76489: // Noise: SN76489
      if (phase > 0x4000'0000) {
        phase -= 0x4000'0000;
        noise = voice_noise_sn76489(&lfsr);
      }
      sample = noise;
      break;
    case gbWaveNoiseWhite: // Noise: White Noise, frequency independent
      lcg *= 1664525;
      lcg += 1013904223;
      sample = lcg & 0x0FFF'FFFF;
      break;
    }

    time++;

    // Apply combined gain (volume * envelope) in single operation
    sample = (sample >> 8) * combinedGain;

    // Apply bitcrush
    if (bitcrush) {
      sample &= (0xffff'ffff << bitcrush);
      // drive is ignored at the moment
      // sample *= drive;
    }

    // Apply panning

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
    frequency = frequencyTable[fIndex];
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
    vibSwing = frequencyTable[fIndex + 1] - frequency;
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

  static inline uint32_t voice_noise_lfsr(uint16_t *lfsr, int b1,
                                          int feedback) {
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

  /* command implementation ***************************************************/

  // integer log2 approximation for Q16.16 input
  static inline int32_t log2_fixed(uint32_t xQ16) {
    if (xQ16 == 0)
      return 0;

    int leading = 31 - __builtin_clz(xQ16); // integer part of log2
    uint32_t frac = xQ16 << (31 - leading); // normalized fractional part
    // take top 16 bits as fraction
    int32_t frac16 = frac >> 15;
    return (leading - 16) << 16 | (frac16 & 0xFFFF); // Q16.16
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
    /*
    performs an exponential*) pitch slide from previous note value to pitch bb
    at speed aa.

    00 is the fastest speed for aa (instant, useless)
    bb values are relative: 00-7F are up, 80-FF are down, expressed in
    semi-tones if LEG is put on a row where a note is present and the pitch
    offset is 0 (e.g. C4 I3 LEG 1000) the slide will occur automatically from
    previous note to the current one at the given speed. If an instrument is not
    triggered on the same row as LEG, the command will re-trigger the previous
    instrument (unless the previous instrument is still playing). LEG does
    exponential pitch change (i;e. it goes at same speed through all octaves)
    while PITCH is linear

    *) it's not exponential in the GameBoy instrument, for performance reasons
    */

    int ticks = 1 + speed; // minimum 1 tick

    if (semitones == 0) {
      // Slide from lastFrequency to current frequency
      if (lastFrequency == frequency) {
        // Same frequency - no slide needed
        legato = false;
        return;
      }

      // Initial factor = lastFrequency / frequency (e.g., 0.5 for up an octave)
      // Target factor = 1.0 (when we reach the new frequency)
      int64_t initialFactor = ((int64_t)lastFrequency << 16) / frequency;
      legatoTargetFreq = 0x0001'0000; // 1.0 in Q16.16
      legatoCoefficient = (legatoTargetFreq - initialFactor) / ticks;
      legatoFactor = initialFactor; // Start at the ratio
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

    for (uint32_t i = 0; i < arpLength - 1; i++) {
      uint8_t semitone = (value >> (i * 4)) & 0x0F;
      int32_t noteVal = note + parameters.transpose + semitone + 12;
      noteVal = (noteVal < 0) ? 0 : noteVal;
      noteVal = (noteVal > 127 + 24) ? 127 + 24 : noteVal;
      arpFrequencies[i + 1] = frequencyTable[noteVal];
    }
  }
} voice_t;