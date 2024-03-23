#ifndef PICO_SYNTH_H
#define PICO_SYNTH_H

#include <cstdint>

const int SAMPLE_RATE = 44100;
const int BUFFER_SIZE = 256;
const int UPDATE_RATE = SAMPLE_RATE / BUFFER_SIZE;

const int HARMONICS = 6;
const int LUT_SIZE = 100;

float noteToFreq(char note);

struct picosynth_env {
  char type;
  int attack;
  int decay;
  int sustain;
  int release;
  int amplitude;
  int phase;
};

class PicoSynth {
public:
  PicoSynth() {}

  void setEnvelopeConfig(char index, picosynth_env config);

  picosynth_env getEnvelopeConfig(char index);

  void generateWaves(uint8_t *byte_stream, int len);

  void update_envelopes();

  void set_defaults();

  void envelope_gate(bool on);

  void set_note(char note);

  char get_note();

private:
  picosynth_env env[HARMONICS];

  // The current per harmonic wave settings
  // wave[h][0] is oscillator amplitude
  // wave[h][1] is initialise the phase (offset into the sine LUT) of the wave

  /*
   * The current envelope settings. This represents the volume of the harmonic,
   * and the state it is in.
   */
  int filt[HARMONICS] = {0, 0, 0, 0, 0, 0};
  char filt_state[HARMONICS] = {0, 0, 0, 0, 0, 0};
  float phase_int[HARMONICS] = {0, 0, 0, 0, 0, 0};

  char gate = 0;
  int _note = 60; /* integer representing midi notes */

  int generatePhaseSample(float phase_increment, float &phase_index, int vol);
};

#endif