#include "Scale.h"
#include <cstdint>

// Source of scales in original release:
// https://upload.wikimedia.org/wikipedia/commons/thumb/3/35/PitchConstellations.svg/1280px-PitchConstellations.svg.png

// const int numScales = 44;

// Note names for the scale root (C, C#, D, etc.)
const char *noteNames[12] = {"C",  "C#", "D",  "D#", "E",  "F",
                             "F#", "G",  "G#", "A",  "A#", "B"};

const char *scaleNames[numScales] = {"None (Chromatic)",
                                     "Acoustic",
                                     "Adonal malakh",
                                     "Aeolian mode (minor)",
                                     "Algerian",
                                     "Altered",
                                     "Augmented",
                                     "Bebop dominant",
                                     "Blues",
                                     "Dorian",
                                     "Double harmonic",
                                     "Enigmatic",
                                     "Flamenco",
                                     "Gypsy",
                                     "Half diminished",
                                     "Harmonic major",
                                     "Harmonic minor",
                                     "Hirajoshi",
                                     "Hungarian gypsy",
                                     "Hungarian minor",
                                     "Insen",
                                     "Ionian mode (major)",
                                     "Istrian",
                                     "Iwato",
                                     "Locrian",
                                     "Lydian augmented",
                                     "Lydian",
                                     "Major bebop",
                                     "Major locran",
                                     "Major pentatonic",
                                     "Melodic minor",
                                     "Melodic minor (asc)",
                                     "Minor pentatonic",
                                     "Mixolydian",
                                     "Neapolitan major",
                                     "Neapolitan minor",
                                     "Octatonic",
                                     "Persian",
                                     "Phrygian dominant",
                                     "Phrygian",
                                     "Prometheus",
                                     "Tritone",
                                     "Ukranian",
                                     "Whole tone"};

const bool scaleSteps[numScales][12] = {
    {true, true, true, true, true, true, true, true, true, true, true, true},
    {true, false, true, false, true, false, true, true, false, true, true,
     false},
    {true, false, true, false, true, true, false, true, true, false, true,
     false},
    {true, false, true, true, false, true, false, true, true, false, true,
     false},
    {true, false, true, true, false, false, true, true, true, false, false,
     true},
    {true, true, false, true, true, false, true, false, true, false, true,
     false},
    {true, false, false, true, true, false, false, true, true, false, false,
     true},
    {true, false, true, false, true, true, false, true, false, true, true,
     true},
    {true, false, false, true, false, true, true, true, false, false, true,
     false},
    {true, false, true, true, false, true, false, true, false, true, true,
     false},
    {true, true, false, false, true, true, false, true, true, false, false,
     true},
    {true, true, false, false, true, false, true, false, true, false, true,
     true},
    {true, true, false, false, true, true, false, true, true, false, false,
     true},
    {true, false, true, true, false, false, true, true, true, false, true,
     false},
    {true, false, true, true, false, true, true, false, true, false, true,
     false},
    {true, false, true, false, true, true, false, true, true, false, false,
     true},
    {true, false, true, true, false, true, false, true, true, false, false,
     true},
    {true, false, true, true, false, false, false, true, true, false, false,
     false},
    {true, false, true, true, false, false, true, true, true, false, false,
     true},
    {true, false, true, true, false, false, true, true, true, false, false,
     true},
    {true, true, false, false, false, true, false, true, false, false, true,
     false},
    {true, false, true, false, true, true, false, true, false, true, false,
     true},
    {true, true, true, true, true, true, false, false, false, false, false,
     false},
    {true, true, false, false, false, true, true, false, false, false, true,
     false},
    {true, true, false, true, false, true, true, false, true, false, true,
     false},
    {true, false, true, false, true, false, true, false, true, true, false,
     true},
    {true, false, true, false, true, false, true, true, false, true, false,
     true},
    {true, false, true, false, true, true, false, true, true, true, false,
     true},
    {true, false, true, false, true, false, true, true, false, true, true,
     false},
    {true, false, true, false, true, false, false, true, false, true, false,
     false},
    {true, false, true, true, false, true, false, true, true, false, true,
     false},
    {true, false, true, true, false, true, false, true, false, true, false,
     true},
    {true, false, false, true, false, true, false, true, false, false, true,
     false},
    {true, false, true, false, true, true, false, true, false, true, true,
     false},
    {true, true, false, true, false, true, false, true, false, true, false,
     true},
    {true, true, false, true, false, true, false, true, true, false, false,
     true},
    {true, false, true, true, false, true, true, false, true, true, false,
     true},
    {true, true, false, false, true, true, true, false, true, false, false,
     true},
    {true, true, false, false, true, true, false, true, true, false, true,
     false},
    {true, true, false, true, false, true, false, true, true, false, true,
     false},
    {true, false, true, false, true, false, true, false, false, true, true,
     false},
    {true, true, false, false, true, false, true, true, false, false, true,
     false},
    {true, false, true, true, false, false, true, true, false, true, true,
     false},
    {true, false, true, false, true, false, true, false, true, false, true,
     false}};

// Return the offset from the root note in semitones for the given scale and
// "scale number", taking into account the scale root
uint8_t getSemitonesOffset(unsigned char scale, unsigned char number,
                           unsigned char root) {
  // Find the nth note in the scale (where n is the number parameter)
  unsigned char i = 0;
  unsigned char foundNotes = 0;

  // Handle the case where we're looking for the root note (first note in scale)
  if (number == 0) {
    return 0;
  }

  // Find the nth note in the scale
  while (foundNotes < number) {
    i++;
    // Adjust for the root note by shifting the scale pattern
    // For root = 0 (C), this simplifies to just checking scaleSteps[scale][i %
    // 12]
    if (scaleSteps[scale][(i + 12 - root) % 12]) {
      foundNotes++;
    }
  }

  return i;
}
