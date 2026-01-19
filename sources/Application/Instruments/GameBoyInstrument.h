/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 * Copyright (c) 2925 nILS Podewski
 *
 * This file is part of the picoTracker firmware
 */

#pragma once

#include "Application/Model/Song.h"
#include "Application/Persistency/PersistenceConstants.h"
#include "I_Instrument.h"
#include <cstdint>

#define GB_NUM_WAVEFORMS 8

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
  uint32_t burstTime;
  uint16_t vibPhase;
  const uint16_t vibFrequency = 0xfff;
  int32_t vibDepth;
  int32_t vibSwing;
  uint32_t vibDelay;

  FourCC command;

  uint32_t sweepCoefficient;
  int32_t sweepSteps;
  uint16_t lfsr = 17;
  uint32_t noise;

  uint32_t lastSample = 0;
  int32_t maxStep = 0x3fff'ffff;
  int32_t minStep = -0x3fff'ffff;

  Envelope envelope;

  void note_on(unsigned char note, bool retrigger) {
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

    // reset vibrato
    vibSwing = frequencyTable[fIndex + 1] - frequency;
    vibDelay = parameters.vibratoDelay << 8;
    vibDepth = parameters.vibratoDepth;
    vibPhase = 0;

    // reset envelope
    envelope.setAttack(parameters.attack);
    envelope.setDecay(parameters.decay);
    envelope.trigger();

    int len = parameters.length;
    lifetime = len ? len : 0xFFFF'FFFF;

    wave = parameters.wave;
    volume = parameters.level;
    burstTime = parameters.burst;

    // sweep
    int32_t sweepDepth = parameters.sweepAmount;
    sweepCoefficient = (1 << 16) + (sweepDepth * 64);
    sweepSteps = parameters.sweepTime;
  }
} voice_t;


constexpr int SAMPLING_RATE = 44100;

constexpr int BASE_MIDI = 69; // A4
constexpr double BASE_FREQ = 440.0;
constexpr int PHASE_BITS = 32;

class GameBoyInstrument : public I_Instrument {

public:
  GameBoyInstrument();
  virtual ~GameBoyInstrument();

  virtual bool Init();

  // Start & stop the instument
  virtual bool Start(int channel, unsigned char note, bool retrigger = true);
  virtual void Stop(int channel);

  // size refers to the number of samples
  // should always fill interleaved stereo / 16bit
  virtual bool Render(int channel, fixed *buffer, int size, bool updateTick);
  virtual void ProcessCommand(int channel, FourCC cc, ushort value);

  virtual bool IsInitialized();

  virtual bool IsEmpty() { return false; };

  virtual InstrumentType GetType() { return IT_GAMEBOY; };

  virtual void OnStart();

  virtual void Purge() {};

  virtual int GetTable();
  virtual bool GetTableAutomation();
  virtual void GetTableState(TableSaveState &state);
  virtual void SetTableState(TableSaveState &state);
  etl::ilist<Variable *> *Variables() { return &variables_; };

  void setChannel(uint8_t channel);

private:
  static voice_t voices_[SONG_CHANNEL_COUNT];
  
  etl::list<Variable *, 13> variables_;

  Variable vWaveform_;
  Variable vAttack_;
  Variable vDecay_;
  Variable vLevel_;
  Variable vLength_;
  Variable vBurst_;
  Variable vVibratoDepth_;
  Variable vVibratoDelay_;
  Variable vTranspose_;
  Variable vTable_;
  Variable vArpSpeed_;
  Variable vSweepTime_;
  Variable vSweepAmount_;


  void RunCommand(int channel);
  void CommandInitArp(int channel, ushort value);
  inline uint32_t pulse(int channel, bool level);
  InstrumentParameters getInstrumentParameters();
};