/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 nILS Podewski
 *
 * This file is part of the picoTracker firmware
 */

#pragma once

#include "Application/Utils/fixed.h"
#include <cstdint>

#include "ChiptuneEnums.h"
#include "ChiptuneEnvelope.h"
#include "ChiptuneMath.h"
#include "ChiptuneTables.h"

#define SAMPLE_LEVEL 0x0FFF'FFFF
#define HALF_SAMPLE_LEVEL (SAMPLE_LEVEL >> 1)
#define NOISE_PHASE_LENGTH 0x4000'0000

/******************************************************************************
 * InstrumentParameters holds all relevant information from the instrument    *
 * view that is needed by the engine.                                         *
 ******************************************************************************/

typedef struct InstrumentParameters {
  // maximum length the voice can reach to cut off a voice before the decay
  // reaches 0. 0 means do not limit
  uint8_t length;

  // waveform enum
  chiptune_wave_type_e wave;
  // envelope attack and decay time (0-255, where 0 is instant)
  uint8_t attack;
  uint8_t decay;
  // instrument output level
  uint8_t level;
  // plays a burst of white noise for the given time at the beginning of a note
  uint8_t burst;
  // vibrato depth and the delay before the automatic vibrato starts
  uint8_t vibratoDepth;
  uint8_t vibratoDelay;
  // instrument transposition (-24..24)
  int8_t transpose;
  // speed at which the arpeggio cycles through the steps (0-35, where 0 is
  // fastest)
  uint8_t arpSpeed;
  // sweep time and amount (amount is in semitones, positive or negative)
  uint8_t sweepTime;
  int8_t sweepAmount;
} InstrumentParameters;

// make sure any type changes do not break the target size or lead to alignment
// issues
static_assert(sizeof(InstrumentParameters) <= 12,
              "Check sizeof(InstrumentParameters) in error message");

/******************************************************************************
 * voice                                                                      *
 ******************************************************************************/

// (!) alignment has to be manually kept in this struct to allow using pack()
//     to keep the size <=128 bytes per voice
#pragma pack(push, 1)
typedef struct voice_t {
  InstrumentParameters parameters;

  uint32_t phase = 0;    // oscillator phase
  int32_t frequency = 0; // precomp'd oscillator frequency (incl. vibrato, etc)
  int32_t lastFrequency = 0; // for legato slides, to compute the initial factor

  uint32_t timeToLive; // time before the voice is silenced

  uint32_t lastSample = 0; // used for both the last sample for pulse smoothing
                           // and as the lcg register for the noise

  struct arp {
    int32_t frequencies[5] = {0, 0, 0, 0, 0}; // precomp'd frequencies
    uint8_t clock = 0;  // internal clock for arpeggio timing
    uint8_t time = 250; // arpeggio step duration in clock ticks
    uint8_t length = 5; // number of steps in the arpeggio (1-5)
    uint8_t index = 0;  // current index in the arpeggio sequence

    inline void tick() {
      clock++;

      if (clock >= time) {
        clock = 0;
        index++;
        if (index >= length) {
          index = 0;
        };
      }
    }
  } arp;

  struct volume {
    uint16_t current; // current volume level (0-65535 Q8.8)
    int16_t step;     // per-tick volume change for slides
    uint8_t level;    // current volume level (0-255) from instrument
    uint8_t target;   // target volume level for slide commands

    inline bool tick() {
      if (level == target) {
        return false; // no slide in progress
      }
      if (step < 0) {
        current = std::max((int32_t)current + step, (int32_t)(target << 8));
      } else {
        current = std::min((int32_t)current + step, (int32_t)(target << 8));
      }

      level = current >> 8;
      return true;
    }
  } volume;

  struct vibrato {
    uint16_t phase; // sine lfo phase
    int32_t swing;  // frequency diff between current note and next semitone
    uint16_t frequency = 0x0FFF; // vibrato frequency
    uint16_t delay;              // ticks before auto-vibrato starts
    uint8_t depth;               // vibrato depth to apply

    int tick(uint32_t time) {
      if (time > delay) {
        phase += frequency;
        int32_t sine = interpolateS8(sine64LUT.data(), phase >> 8);
        int delta = (swing * sine) >> 7;
        delta = (depth * delta) >> 8;
        return delta;
      }
      return 0;
    }
  } vibrato;

  struct pan {
    uint8_t position; // current pan position (0-255, 128=center)
    uint8_t target;   // target pan position for slew
    int8_t step;      // slew step size

    inline void tick() {
      if (step == 0)
        return;

      position += step;

      // check if we've reached or overshot the target
      if ((step > 0 && position >= target) ||
          (step < 0 && position <= target)) {
        position = target;
        step = 0;
      }
    }
  } pan;

  struct sweep {
    uint32_t coefficient; // addition coefficient to scale
    int16_t steps;        // number of times to apply
  } sweep;

  uint8_t burstTime;   // initial white noise burst duration in ticks
  chiptuneFlags flags; // voice command flags

  struct legato {
    // legato (pitch slide)
    int32_t coefficient = 0;  // q16.16 multiplier per 100Hz tick
    int32_t factor = 0;       // q16.16 multiplier per 100Hz tick
    int32_t steps = 0;        // remaining ticks
    int32_t targetFactor = 0; // target multiplication factor

    inline void tick() {
      if (steps) {
        // accumulate factor: legato.factor *= legato.coefficient (both Q16.16)
        factor += coefficient;
        steps--;

        if (steps == 0) {
          factor = targetFactor;
        }
      }
    }
  } legato;

  uint8_t drive;    // unused currently
  uint8_t bitcrush; // bitcrush setting (only settable via command)

  uint8_t note;              // current base note
  chiptune_wave_type_e wave; // selected waveform

  uint16_t lfsr = 17; // shift register for the noise generators

  envelope_t envelope; // envelope, size is 9 bytes

  struct gain {
    uint8_t left;     // gain setting for the left channel
    uint8_t right;    // gain setting for the right channel
    uint8_t combined; // master gain (precomputed at tock rate)
  } gain;

  inline void calculate_gain() {
    gain.left = std::min((0xFF - pan.position) * 2, 0xFF);
    gain.right = std::min(0xFF, 2 * pan.position);
    gain.combined = (volume.level * envelope.value) >> 16;
  }

  uint16_t tick; // sample counter for 100Hz updates
  uint32_t time; // sample counter
  uint8_t tock;  // sample counter for 1000Hz updates

  uint8_t alignmentSentinel[3]; // placeholder to guarantee alignment & padding

  // implementation ------------------------------------------------------------

  inline void stop() {
    frequency = 0;
    phase = 0;
  }

  inline void tick_100Hz() {
    // envelope processing at ~100Hz
    envelope.tick();

    if (flags.volume) {
      flags.volume = volume.tick();
    }

    // sweep
    if (sweep.steps) {
      sweep.steps--;

      // sweep all base frequencies
      for (uint32_t i = 0; i < arp.length; i++) {
        uint64_t f = arp.frequencies[i];
        f *= sweep.coefficient;
        arp.frequencies[i] = uint32_t(f >> 16);
      }
    }

    // pan
    if (pan.step) {
      pan.tick();
    }

    // recompute combined gain when envelope, pan or volume changes
    calculate_gain();

    // vibrato
    frequency = arp.frequencies[arp.index] + vibrato.tick(time);

    // legato: frequency interpolation (Q16.16 fixed-point)
    if (flags.legato) {
      legato.tick();
    }

    // apply to frequency: frequency *= legato.factor (Q16.16)
    frequency = ((int64_t)frequency * legato.factor) >> 16;
  }

  inline void tick_1000Hz() {
    // arpeggio
    if (flags.arpeggio) {
      arp.tick();
    }

    if (timeToLive == 0) {
      if (flags.retrigger) {
        flags.retrigger = 0; // clear retrigger flag
        // retrigger without resetting clocks
        note_on(note, false, parameters, true);
      } else {
        // note off, kill everything
        wave = waveNone;
        envelope.state = envIdle;
      }
    } else {
      // still in burst or already playing?
      if (burstTime > 0) {
        burstTime--;
        wave = waveNoiseWhite;
        flags.burst_end = (burstTime == 0);
      } else {
        // first non-burst tick, reset phase, last sample, etc.
        if (flags.burst_end) {
          flags.burst_end = 0;
          lastSample = 0;
          phase = 0;
        }

        wave = parameters.wave;
      }

      // length
      timeToLive--;
    }
  }

  inline void sample(fixed *left, fixed *right) {
    // precompute the gain, it doesn't need to be updated every sample

    // cold loop @ 100 Hz ------------------------------------------------------
    if (tick == 0) {
      tick = ticks100Hz;
      tick_100Hz();
    }

    // warm loop @ ~1000 Hz ----------------------------------------------------
    if (tock == 0) {
      tock = ticks1000Hz;
      tick_1000Hz(); // update at 1kHz for smoother pan and volume slides
    }

    // hot loop @ ~44100 Hz ----------------------------------------------------
    tick--;
    tock--;
    time++;

    // advance phase
    phase += frequency;

    uint32_t sample = 0;

    // generate sample based on waveform
    switch (wave) {
    case wavePulse12_5: // pulse 12.5%
      sample = pulse(phase > 0x2000'0000);
      break;
    case wavePulse25: // pulse 25%
      sample = pulse(phase > 0x4000'0000);
      break;
    case wavePulse50: // pulse 50%
      sample = pulse(phase > 0x8000'0000);
      break;
    case waveTriangle:           // triangle
      if (phase < 0x8000'0000) { // first half, rising slope
        sample = phase >> 3;
      } else { // second half, falling slope
        sample = (0xFFFF'FFFF - phase) >> 3;
      }
      sample &= 0xFF00'0000; // downsample
      break;
    case waveNoiseGameBoy7: // noise: GB7
      sample = voice_noise_lfsr(1, 6);
      break;
    case waveNoiseNES: // noise: NES
      sample = voice_noise_lfsr(6, 14);
      break;
    case waveNoiseSN76489: // noise: SN76489
      sample = voice_noise_lfsr(3, 14);
      break;
    case waveNoiseWhite:                                // noise: white noise
      lastSample = (lastSample * 1664525) + 1013904223; // frequency independent
      sample = lastSample & SAMPLE_LEVEL;
      break;
    case waveNone:
      sample = 0;
      break;
    }

    // apply combined gain (volume * envelope) in single operation
    sample = (sample >> 8) * gain.combined;

    // apply bitcrush
    if (bitcrush) {
      sample &= (0xffff'ffff << bitcrush);
      // drive is ignored at the moment
      // sample *= drive;
    }

    // apply panning
    *left = (sample >> 8) * gain.left;
    *right = (sample >> 8) * gain.right;
  }

  inline void note_on(unsigned char note, bool retrigger,
                      const InstrumentParameters &parameters,
                      bool keepClocks = false) {
    // bool retrigger is currently unused
    this->parameters = parameters;

    // is this the best time to store that?
    lastFrequency = frequency;

    // command settings
    legato.steps = 0;
    legato.factor = q16_16_1;      // 1.0 in Q16.16
    legato.coefficient = q16_16_1; // 1.0 in Q16.16
    flags.byte = 0;                // clear all flags

    bitcrush = 0; // only accessible via command
    drive = 0;

    // reset panning
    pan.position = 128;
    pan.target = 128;
    pan.step = 0;

    // reset volume slide
    volume.level = parameters.level;
    volume.target = parameters.level;
    volume.current = parameters.level << 8;
    volume.step = 0;

    // oscillator frequency setup
    int fIndex = std::clamp(note + 12 + parameters.transpose, 0, 127 + 24);
    frequency = frequencyLUT[fIndex];
    arp.frequencies[0] = frequency;
    wave = parameters.wave;
    burstTime = parameters.burst;

    // reset noise seed to get deterministic noise
    lfsr = 42;
    //    noise = 67;
    lastSample = 42;

    this->note = note;

    // reset arpepggio to default state (off, base frequency only)
    arp.time = 35 - parameters.arpSpeed;
    arp.index = 0;
    arp.clock = 0;

    // reset oscillator state and timers
    timeToLive = (parameters.length == 0) ? 0x7FFF'FFFF : (parameters.length);
    phase = 0;

    // don't reset timers on internal retrigger via command (IRT, ...)
    // they might be mid-execution and will underflow
    if (!keepClocks) {
      time = 0;
      tick = 0;
      tock = 0;
    }

    // reset vibrato
    vibrato.depth = parameters.vibratoDepth;
    vibrato.swing = frequencyLUT[fIndex + 1] - frequency;
    vibrato.delay = parameters.vibratoDelay << 8;
    vibrato.phase = 0;
    vibrato.frequency = vibratoFrequency;

    // reset envelope
    envelope.set_attack(parameters.attack);
    envelope.set_decay(parameters.decay);
    envelope.trigger();

    // sweep
    int32_t sweepDepth = parameters.sweepAmount;
    sweep.coefficient = (1 << 16) + (sweepDepth * 64);
    sweep.steps = parameters.sweepTime;

    // precalculate the gain
    calculate_gain();
  }

  /****************************************************************************
   *  waveform generation                                                     *
   ****************************************************************************/

  // simple slew limited pulse generator to avoid some of the aliasing noise
  // while keeping the implementation fast enough for 8 voices on rp2040
  inline uint32_t pulse(bool high) {
    int32_t target = high ? SAMPLE_LEVEL : 0;
    int32_t step = frequency + 0x1FFF'FFFF; // 0.125 + phase increment
    int32_t diff = std::clamp(target - (int32_t)lastSample, -step, step);
    return (lastSample = (lastSample + diff));
  }

  // LFSR function for noise generation, parameterized by tap and feedback bits
  inline uint32_t voice_noise_lfsr(uint8_t bit, uint8_t feedback) {
    // only resample noise when the phase is > 25% to get frequency-dependent
    // noise
    if (phase > NOISE_PHASE_LENGTH) {
      phase -= NOISE_PHASE_LENGTH;

      uint32_t bitA = lfsr & 1;
      uint32_t bitB = (lfsr >> bit) & 1;
      uint32_t bitF = bitA ^ bitB;

      lfsr = (lfsr >> 1) | (bitF << feedback);
      lastSample = bitA ? 0x0FFF'FFFF : 0;
    }

    return lastSample;
  }

  /****************************************************************************
   *  command processing                                                     *
   ****************************************************************************/

  void command_init_instrument_retrigger(uint8_t delay, int8_t transpose) {
    flags.retrigger = 1; // set retrigger flag
    timeToLive = delay;  // make sure the note is killed in the next tick
    note += transpose;
  }

  void command_init_pan(uint8_t speed, int8_t target) {
    pan.target = target;
    pan.step = (target > pan.position) ? speed : -speed;
  }

  void command_init_vibrato(uint8_t rate, uint8_t depth) {
    vibrato.frequency = rate << 6;

    int fIndex = std::clamp(note + 12 + parameters.transpose, 0, 127 + 24);
    vibrato.swing = frequencyLUT[fIndex + 1] - frequency;
    vibrato.depth = depth;
    vibrato.delay = 0; // vibrato starts immediately on command
  }

  void command_init_finetune(uint8_t rate, int8_t amount) {
    // finetune uses the same mechanism as legato
    int32_t offset = semitoneRatioQ16[128 + sign(amount)];
    offset = (offset * amount) >> 7;

    // not exponential in the Chiptune instrument, for performance reasons (atm)
    int ticks = 1 + rate; // minimum 1 tick

    // get total ratio from table (Q16.16)
    legato.targetFactor = offset;
    legato.coefficient = (legato.targetFactor - 0x0001'0000) / ticks;

    legato.steps = ticks;
    legato.factor = 0x0001'0000; // start at 1.0 in q16.16

    flags.legato = 1; // set legato flag
  }

  void command_init_volume(uint8_t duration, uint8_t target) {
    // volume slide
    volume.target = target;

    int ticks = std::max(1, (int)duration);
    int delta = (int(target) << 8) - int(volume.current);
    volume.step = delta / ticks;

    if ((volume.step == 0) && (delta != 0)) {
      volume.step = sign(delta);
    }

    flags.volume = 1; // set volume slide flag
  }

  // fully fixed-point per-tick legato initialization
  void command_init_legato(uint8_t speed, int8_t semitones) {
    // not exponential in the Chiptune instrument, for performance reasons (atm)
    int ticks = 1 + speed; // minimum 1 tick

    if (semitones == 0) {
      // slide from lastFrequency to current frequency
      if (lastFrequency == frequency) {
        // same frequency - no slide needed
        flags.legato = 0; // clear legato flag
        return;
      }

      // initial factor = lastFrequency / frequency (0.5 for up an octave etc)
      // target factor = 1.0 (when we reach the new frequency)
      int64_t initialFactor = ((int64_t)lastFrequency << 16) / frequency;
      legato.targetFactor = 0x0001'0000; // 1.0 in Q16.16
      legato.coefficient = (legato.targetFactor - initialFactor) / ticks;
      legato.factor = initialFactor; // start at the ratio
    } else {
      // get total ratio from table (Q16.16)
      legato.targetFactor = semitoneRatioQ16[semitones + 128];
      legato.coefficient = (legato.targetFactor - 0x0001'0000) / ticks;
    }

    legato.steps = ticks;
    legato.factor = 0x0001'0000; // start at 1.0

    flags.legato = 1; // set legato flag
  }

  // fully fixed-point per-tick pitch shift initialization
  void command_init_pitch_shift(uint8_t speed, int8_t semitones) {
    // not exponential in the Chiptune instrument, for performance reasons (atm)
    int ticks = 1 + speed; // minimum 1 tick

    // get total ratio from table (Q16.16)
    legato.targetFactor = semitoneRatioQ16[semitones + 128];
    legato.coefficient = (legato.targetFactor - 0x0001'0000) / ticks;

    legato.steps = ticks;
    legato.factor = 0x0001'0000; // start at 1.0 in q16.16

    flags.legato = 1; // set legato flag
  }

  void command_init_arp(uint16_t value) {
    // preset to full length arpeggio
    arp.index = 0;
    arp.length = 5;

    // trim off trailing zeroes
    uint16_t val = value;

    while ((arp.length > 1) && !(val & 0x000F)) {
      arp.length--;
      val >>= 4;
    }

    // calculate frequencies for each step in the arpeggio
    for (uint8_t i = 0; i < arp.length - 1; i++) {
      uint8_t semitone = (value >> (12 - i * 4)) & 0x000F;
      int32_t noteVal = note + parameters.transpose + semitone + 12;
      noteVal = (noteVal < 0) ? 0 : noteVal;
      noteVal = (noteVal > 127 + 24) ? 127 + 24 : noteVal;
      arp.frequencies[i + 1] = frequencyLUT[noteVal];
    }

    flags.arpeggio = 1; // set arpeggio flag
  }
} voice_t;
#pragma pack(pop)

// 128 bytes per voice max to keep the entire thing under 1kB for the 8 voices,
// also struct needs to be aligned to 4 bytes to prevent unaligned access
static_assert(sizeof(voice_t) <= 128, "Check sizeof(voice_t) in error message");
static_assert((sizeof(voice_t) % 4) == 0, "voice_t size must be multiple of 4");