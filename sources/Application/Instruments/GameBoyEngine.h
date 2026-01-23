#pragma once

#include "Application/Utils/fixed.h"
#include "System/Console/Trace.h"
#include <cstdint>

static uint32_t lcg = 42;
#define GB_NUM_WAVEFORMS 8

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

// precalculated note step values for DDS (32-bit phase accumulator)
static const uint32_t semitoneRatioQ16[256] = {
    0x00000028, 0x0000002A, 0x0000002D, 0x0000002F, 0x00000032, 0x00000035,
    0x00000039, 0x0000003C, 0x00000040, 0x00000043, 0x00000047, 0x0000004C,
    0x00000050, 0x00000055, 0x0000005A, 0x0000005F, 0x00000065, 0x0000006B,
    0x00000072, 0x00000078, 0x00000080, 0x00000087, 0x0000008F, 0x00000098,
    0x000000A1, 0x000000AA, 0x000000B5, 0x000000BF, 0x000000CB, 0x000000D7,
    0x000000E4, 0x000000F1, 0x00000100, 0x0000010F, 0x0000011F, 0x00000130,
    0x00000142, 0x00000155, 0x0000016A, 0x0000017F, 0x00000196, 0x000001AE,
    0x000001C8, 0x000001E3, 0x00000200, 0x0000021E, 0x0000023E, 0x00000260,
    0x00000285, 0x000002AB, 0x000002D4, 0x000002FF, 0x0000032C, 0x0000035D,
    0x00000390, 0x000003C6, 0x00000400, 0x0000043C, 0x0000047D, 0x000004C1,
    0x0000050A, 0x00000556, 0x000005A8, 0x000005FE, 0x00000659, 0x000006BA,
    0x00000720, 0x0000078D, 0x00000800, 0x00000879, 0x000008FA, 0x00000983,
    0x00000A14, 0x00000AAD, 0x00000B50, 0x00000BFC, 0x00000CB2, 0x00000D74,
    0x00000E41, 0x00000F1A, 0x00001000, 0x000010F3, 0x000011F5, 0x00001306,
    0x00001428, 0x0000155B, 0x000016A0, 0x000017F9, 0x00001965, 0x00001AE8,
    0x00001C82, 0x00001E34, 0x00002000, 0x000021E7, 0x000023EB, 0x0000260D,
    0x00002851, 0x00002AB7, 0x00002D41, 0x00002FF2, 0x000032CB, 0x000035D1,
    0x00003904, 0x00003C68, 0x00004000, 0x000043CE, 0x000047D6, 0x00004C1B,
    0x000050A2, 0x0000556E, 0x00005A82, 0x00005FE4, 0x00006597, 0x00006BA2,
    0x00007208, 0x000078D0, 0x00008000, 0x0000879C, 0x00008FAC, 0x00009837,
    0x0000A145, 0x0000AADC, 0x0000B504, 0x0000BFC8, 0x0000CB2F, 0x0000D744,
    0x0000E411, 0x0000F1A1, 0x00010000, 0x00010F38, 0x00011F59, 0x0001306F,
    0x0001428A, 0x000155B8, 0x00016A09, 0x00017F91, 0x0001965F, 0x0001AE89,
    0x0001C823, 0x0001E343, 0x00020000, 0x00021E71, 0x00023EB3, 0x000260DF,
    0x00028514, 0x0002AB70, 0x0002D413, 0x0002FF22, 0x00032CBF, 0x00035D13,
    0x00039047, 0x0003C686, 0x00040000, 0x00043CE3, 0x00047D66, 0x0004C1BF,
    0x00050A28, 0x000556E0, 0x0005A827, 0x0005FE44, 0x0006597F, 0x0006BA27,
    0x0007208F, 0x00078D0D, 0x00080000, 0x000879C7, 0x0008FACD, 0x0009837F,
    0x000A1451, 0x000AADC0, 0x000B504F, 0x000BFC88, 0x000CB2FF, 0x000D744F,
    0x000E411F, 0x000F1A1B, 0x00100000, 0x0010F38F, 0x0011F59A, 0x001306FE,
    0x001428A2, 0x00155B81, 0x0016A09E, 0x0017F910, 0x001965FE, 0x001AE89F,
    0x001C823E, 0x001E3437, 0x00200000, 0x0021E71F, 0x0023EB35, 0x00260DFC,
    0x00285145, 0x002AB702, 0x002D413C, 0x002FF221, 0x0032CBFD, 0x0035D13F,
    0x0039047C, 0x003C686F, 0x00400000, 0x0043CE3E, 0x0047D66B, 0x004C1BF8,
    0x0050A28B, 0x00556E04, 0x005A8279, 0x005FE443, 0x006597FA, 0x006BA27E,
    0x007208F8, 0x0078D0DF, 0x00800000, 0x00879C7C, 0x008FACD6, 0x009837F0,
    0x00A14517, 0x00AADC08, 0x00B504F3, 0x00BFC886, 0x00CB2FF5, 0x00D744FC,
    0x00E411F0, 0x00F1A1BF, 0x01000000, 0x010F38F9, 0x011F59AC, 0x01306FE0,
    0x01428A2F, 0x0155B810, 0x016A09E6, 0x017F910D, 0x01965FEA, 0x01AE89F9,
    0x01C823E0, 0x01E3437E, 0x02000000, 0x021E71F2, 0x023EB358, 0x0260DFC1,
    0x0285145F, 0x02AB7021, 0x02D413CC, 0x02FF221A, 0x032CBFD4, 0x035D13F3,
    0x039047C0, 0x03C686FC, 0x04000000, 0x043CE3E4, 0x047D66B0, 0x04C1BF82,
    0x050A28BE, 0x0556E042, 0x05A82799, 0x05FE4435,
};

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

  uint16_t arpTick = 0;
  uint16_t arpTime = 250;
  uint8_t note;
  uint32_t phase = 0;
  int32_t frequency = 0;
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

  FourCC command;

  uint32_t sweepCoefficient;
  int32_t sweepSteps;

  // Legato (exponential pitch slide)
  int32_t legatoCoefficient = 0; // Q16.16 multiplier per 100Hz tick
  int32_t legatoFactor = 0;      // Q16.16 multiplier per 100Hz tick
  int32_t legatoSteps = 0;       // Remaining ticks
  int32_t legatoTargetFreq = 0;  // Target frequency

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

  // convert a floating-point factor to Q0.32 fixed-point
  static inline uint32_t floatToQ32(double f) {
    return (uint32_t)(f * 4294967296.0); // 2^32
  }

  // apply slide factor to DDS step
  static inline uint32_t applySlide(uint32_t step, uint32_t factorQ32) {
    return multiplyQ32(step, factorQ32);
  }

  inline fixed sample() {
    uint32_t combinedGain = (volume * envelope.value) >> 16; // Precompute
    uint32_t waveform = wave;

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

      // vibrato
      int delta = 0;

      if (time > vibDelay) {
        vibPhase += vibFrequency;
        int32_t sine = interpolateS8(sine64, (vibPhase >> 8));
        delta = (vibSwing * sine) >> 7;
        delta = (vibDepth * delta) >> 8;
      }

      frequency = arpFrequencies[arpIndex] + delta;

      // Legato: exponential frequency interpolation
      if (legatoSteps > 0 && command == FourCC::InstrumentCommandLegato) {
        legatoFactor = applySlide(legatoFactor, legatoCoefficient);
        legatoSteps--;
        frequency = multiplyQ32(frequency, legatoFactor);
      }
    }

    // warm loop @ ~1000 Hz ----------------------------------------------------
    if (tock == 0) {
      // hot loop

      // envelope processing at ~1000Hz
      tock = 44;

      // arpeggio
      if (command == FourCC::InstrumentCommandArpeggiator) {
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

    return sample;
  }

  inline void note_on(unsigned char note, bool retrigger,
                      InstrumentParameters parameters) {
    this->parameters = parameters;

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
    legatoCoefficient = 0xFFFF'FFFF; // 1.0 in Q0.32

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

  // fully fixed-point per-tick legato initialization
  void command_init_legato(uint8_t speed, int8_t semitones) {
    /*
    performs an exponential pitch slide from previous note value to pitch bb at
    speed aa.

    00 is the fastest speed for aa (instant, useless)
    bb values are relative: 00-7F are up, 80-FF are down, expressed in
    semi-tones if LEG is put on a row where a note is present and the pitch
    offset is 0 (e.g. C4 I3 LEG 1000) the slide will occur automatically from
    previous note to the current one at the given speed. If an instrument is not
    triggered on the same row as LEG, the command will re-trigger the previous
    instrument (unless the previous instrument is still playing). LEG does
    exponential pitch change (i;e. it goes at same speed through all octaves)
    while PITCH is linear
    */

    int ticks = 1 + speed; // minimum 1 tick

    // clamp semitones to table
    if (semitones < -128)
      semitones = -128;
    if (semitones > 127)
      semitones = 127;

    // get total ratio from table (Q16.16)
    uint32_t ratioQ16 = semitoneRatioQ16[semitones + 128];

    // compute log2 of ratio in Q16.16
    int32_t log2Total = log2_fixed(ratioQ16);

    // divide by ticks to get per-tick log2
    int32_t log2PerTick = log2Total / ticks;

    // compute per-tick factor in Q16.16
    legatoCoefficient = exp2_fixed(log2PerTick); // Q16.16

    // initialize factor to 1.0 in Q16.16
    legatoFactor = 0x00010000;

    // remaining ticks
    legatoSteps = ticks;
  }

  void command_init_arp(ushort value) {
    arpIndex = 0;
    arpLength = 5;

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