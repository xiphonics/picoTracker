#include "picosynth.h"
#include "fixed.h"
#include "math.h"
#include <cstdint>
#include <stdio.h>
#include <string.h>

int16_t sine[LUT_SIZE] = {
    0,      2057,   4106,   6139,   8148,   10125,  12062,  13951,  15785,
    17557,  19259,  20886,  22430,  23886,  25247,  26509,  27666,  28713,
    29648,  30465,  31163,  31737,  32186,  32508,  32702,  32767,  32702,
    32508,  32186,  31737,  31163,  30465,  29648,  28713,  27666,  26509,
    25247,  23886,  22430,  20886,  19259,  17557,  15785,  13951,  12062,
    10125,  8148,   6139,   4106,   2057,   0,      -2057,  -4106,  -6139,
    -8148,  -10125, -12062, -13951, -15785, -17557, -19259, -20886, -22430,
    -23886, -25247, -26509, -27666, -28713, -29648, -30465, -31163, -31737,
    -32186, -32508, -32702, -32767, -32702, -32508, -32186, -31737, -31163,
    -30465, -29648, -28713, -27666, -26509, -25247, -23886, -22430, -20886,
    -19259, -17557, -15785, -13951, -12062, -10125, -8148,  -6139,  -4106,
    -2057,
};

int PicoSynth::generatePhaseSample(float phase_increment, float &phase_index,
                                   int vol) {
  int result = 0;
  phase_index = phase_index;
  phase_index += phase_increment;

  if (phase_index >= LUT_SIZE) {
    int diff = phase_index - LUT_SIZE;
    phase_index = diff;
  }

  result = sine[(int)phase_index];
  int result_fp = i2fp(result);
  auto res_fp = fp_mul(result_fp, vol);
  return fp2i(res_fp);
}

// Calculate frequency from MIDI note value
// ref: https://gist.github.com/YuxiUx/ef84328d95b10d0fcbf537de77b936cd
float noteToFreq(char note) {
  float a = 440; // frequency of A4 (440Hz)
  return (a / 32) * pow(2, ((note - 9) / 12.0));
}

void PicoSynth::setEnvelopeConfig(char index, picosynth_env config) {
  env[index] = config;
}

picosynth_env PicoSynth::getEnvelopeConfig(char index) { return env[index]; }

char env_update_count = 0;
void PicoSynth::generateWaves(uint8_t *byte_stream, int len) {
  int16_t *s_byte_stream;

  if (env_update_count++ % 10) {
    update_envelopes();
  }

  // get correct phase increment for note depending on sample rate and LUT
  // length.
  float baseNoteFreq = (noteToFreq(_note) / SAMPLE_RATE) * LUT_SIZE;

  // Harmonics of primary freq
  float phase_increment[HARMONICS] = {
      baseNoteFreq,     2 * baseNoteFreq, 3 * baseNoteFreq,
      4 * baseNoteFreq, 5 * baseNoteFreq, 6 * baseNoteFreq,
  };

  /* cast buffer as 16bit signed int */
  s_byte_stream = (int16_t *)byte_stream;

  // generate samples
  for (int i = 0; i < len; i++) {

    int fullResult = 0;
    for (int h = 0; h < HARMONICS; h++) {
      fullResult +=
          generatePhaseSample(phase_increment[h], phase_int[h], filt[h]);
    }
    // write sum of all harmonics into audio buffer
    s_byte_stream[i] = fullResult;
  }
}

void PicoSynth::envelope_gate(bool on) {
  char state = on ? 0 : 4;
  for (int h = 0; h < HARMONICS; h++) {
    // retrigger any still playing envelopes
    if (on && filt_state[h] != 6) {
      filt[h] = 0;
    }
    filt_state[h] = state;
  }
}

void PicoSynth::set_note(char note) { _note = note; }

char PicoSynth::get_note() { return _note; }

void PicoSynth::set_defaults() {
  memset(env, 0, sizeof(env));
  // memset(lfo, 0, sizeof(lfo));
  // memset(special, 0, sizeof(special));

  for (int h = 0; h < HARMONICS; h++) {
    env[h].attack = 0;
    env[h].sustain = 4000;
    env[h].release = 0;
  }
  // sawtooth
  // int saw_vol = 5000;
  // env[0].amplitude = saw_vol;
  // env[1].amplitude = saw_vol / 2;
  // env[2].amplitude = saw_vol / 3;
  // env[3].amplitude = saw_vol / 4;
  // env[4].amplitude = saw_vol / 5;
  // env[5].amplitude = saw_vol / 6;

  // Square
  // int square_vol = 4000;
  // env[0].amplitude = square_vol;
  // env[2].amplitude = square_vol / 3;
  // env[4].amplitude = square_vol / 5;

  // Sine
  env[0].amplitude = 15000;

  envelope_gate(0);
}

/*
 * Needs to be called regularly (eg every buffer fill), this updates the
 * individual harmonic volumes in line with their Envelope. There are 4 envelope
 * types. These are ADSR, delayed ADSR, AD repeat and delayed AD repeat. The
 * delayed envelope waits to go into the decay phase until all active
 * oscillators have finished their attack phase.
 *
 * State 0 => attack
 * State 1 => delay
 * State 2 => decay
 * State 3 => sustain
 * State 4 => release
 * State 5 => note off
 *
 * A standard adsr will go, states: 0, 2, 3, 4, 5
 */
void PicoSynth::update_envelopes() {
  u_char finished = 0;
  u_char playing = HARMONICS;
  int attack, decay, level, sustain, rel;
  u_char etype;
  u_char tremolo = 1;

  /*
   * Work out which notes are still playing, and which have
   * finished their attack phase. Repeating envelopes don't
   * count towards a playing note as they repeat indefinitely.
   */
  for (char i = 0; i < HARMONICS; i++) {
    if (env[i].type < 2 && env[i].type)
      // tremolo = 0;
      if (filt_state[i] > 0 || env[i].type > 1) {
        finished++;
        if (filt_state[i] == 5 || env[i].type > 1)
          playing--;
      }
  }

  /*
   * Loop through the oscillators, and set their filt value. The
   * filt value represents the current volume of the oscillator.
   */
  for (u_char i = 0; i < HARMONICS; i++) {
    level = env[i].amplitude;
    if (level == 0) {
      filt_state[i] = 6;
      continue;
    }
    etype = (u_char)(env[i].type);
    attack = env[i].attack;
    decay = env[i].decay;
    sustain = env[i].sustain;
    rel = env[i].release;
    switch (filt_state[i]) {
    case 0: // Attack
      filt[i] += attack;
      if (attack == 0 || filt[i] >= level) {
        filt[i] = level;
        filt_state[i] = (etype & 1) ? 1 : 2;
      }
      break;
    case 1: // Delay
      if (finished == 6)
        filt_state[i] = 2;
      else
        break;
    case 2: // Decay
      filt[i] -= decay;
      if (decay == 0 || filt[i] <= sustain) {
        filt[i] = sustain;
        if (etype > 1 && etype < 4) {
          if (playing || tremolo)
            filt_state[i] = 0;
          else
            filt_state[i] = 6;
        } else {
          filt_state[i] = 3;
        }
      }
      break;
    case 3: // Sustain stay here until told to release
      if (filt[i] == 0)
        filt_state[i] = 6;
      break;
    case 4: // Release
      if (etype == 4) {
        filt[i] += rel;
        if (rel == 0 || filt[i] >= level) {
          filt[i] = level;
          filt_state[i] = 5;
        }
      } else {
        // printf("%dREL[%d]\n", i, filt[i]);
        filt[i] -= rel;
        if (rel == 0 || filt[i] <= 0) {
          filt[i] = 0;
          filt_state[i] = 6;
        }
      }
      break;
    case 5: // Reverse Release
      filt[i] -= decay;
      if (rel == 0 || filt[i] <= 0) {
        filt[i] = 0;
        filt_state[i] = 6;
      }
      break;
    default:       // Note off
      filt[i] = 0; // take care of repeating oscillators
      break;
    }
  }
}