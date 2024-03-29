#ifndef TINY_SYNTH_H
#define TINY_SYNTH_H

#include "fixed.h"
#include <cstdint>
#include <math.h>

const int SAMPLE_RATE = 44100;
const int BUFFER_SIZE = 512;
const int UPDATE_RATE = SAMPLE_RATE / BUFFER_SIZE;

const int HARMONICS = 6;
const int LUT_SIZE = 60;

float noteToFreq(char note);

struct tinysynth_env {
  char type;
  char attack;
  char decay;
  char sustain;
  char release;
  char amplitude;
  char phase;
};

class TinySynth {
public:
  TinySynth() {}

  void setEnvelopeConfig(int8_t index, tinysynth_env config);

  tinysynth_env getEnvelopeConfig(int8_t index);

  void generateWaves(int16_t *byte_stream, int len);

  void update_envelopes();

  void set_defaults();

  void envelope_gate(bool on);

  void set_note(char note);

  char get_note();

private:
  tinysynth_env env[HARMONICS];
  // The current envelope settings. This represents the volume of the harmonic,
  // and the state it is in.
  int filt[HARMONICS] = {0, 0, 0, 0, 0, 0};
  // make sure we start in note-off state
  char filt_state[HARMONICS] = {6, 6, 6, 6, 6, 6};
  float phase_int[HARMONICS] = {0, 0, 0, 0, 0, 0};

  char _note = 60; /* integer representing midi notes */
  char _env_update_count = 0;

  int generatePhaseSample(float phase_increment, float &phase_index, int vol);
};

#endif