#include "Scale.h"
#include "System/Console/n_assert.h"

// Source of scales in original release:
// https://upload.wikimedia.org/wikipedia/commons/thumb/3/35/PitchConstellations.svg/1280px-PitchConstellations.svg.png

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
                                     "Major locrian",
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
    {true, true, true, true, true, true, true, true, true, true, true,
     true}, // None (Chromatic)
    {true, false, true, false, true, false, true, true, false, true, true,
     false}, // Acoustic
    {true, false, true, true, false, true, false, true, false, true, true,
     false}, // Adonal malakh
    {true, false, true, false, true, true, false, true, false, true, false,
     true}, // Aeolian mode (minor)
    {true, true, false, false, true, false, true, true, true, false, false,
     true}, // Algerian
    {true, true, false, true, true, false, true, false, true, false, true,
     false}, // Altered
    {true, false, false, true, true, false, false, true, true, false, false,
     true}, // Augmented
    {true, false, true, false, true, true, false, true, false, true, true,
     true}, // Bebop dominant
    {true, false, false, true, false, true, true, true, false, false, true,
     false}, // Blues
    {true, false, true, true, false, true, false, true, false, true, true,
     false}, // Dorian
    {true, true, false, false, true, true, false, true, true, false, false,
     true}, // Double harmonic
    {true, true, false, false, true, false, true, false, true, false, true,
     true}, // Enigmatic
    {true, true, false, false, true, true, false, true, true, false, false,
     true}, // Flamenco
    {true, false, true, true, false, false, true, true, true, false, true,
     false}, // Gypsy
    {true, false, true, true, false, true, true, false, true, false, true,
     false}, // Half diminished
    {true, false, true, false, true, true, false, true, true, false, false,
     true}, // Harmonic major
    {true, false, true, true, false, true, false, true, true, false, false,
     true}, // Harmonic minor
    {true, false, true, true, false, false, false, true, true, false, false,
     false}, // Hirajoshi
    {true, false, true, true, false, false, true, true, true, false, false,
     true}, // Hungarian gypsy
    {true, false, true, true, false, false, true, true, true, false, false,
     true}, // Hungarian minor
    {true, true, false, false, false, true, false, true, false, false, true,
     false}, // Insen
    {true, false, true, false, true, true, false, true, false, true, false,
     true}, // Ionian mode (major)
    {true, true, false, true, true, false, true, true, false, false, false,
     false}, // Istrian
    {true, true, false, false, false, true, true, false, false, false, true,
     false}, // Iwato
    {true, true, false, true, false, true, true, false, true, false, true,
     false}, // Locrian
    {true, false, true, false, true, false, true, false, true, true, false,
     true}, // Lydian augmented
    {true, false, true, false, true, false, true, true, false, true, false,
     true}, // Lydian
    {true, false, true, false, true, true, false, true, true, true, false,
     true}, // Major bebop
    {true, false, true, false, true, true, true, false, true, false, true,
     false}, // Major locrian
    {true, false, true, false, true, false, false, true, false, true, false,
     false}, // Major pentatonic
    {true, false, true, true, false, true, false, true, true, false, true,
     false}, // Melodic minor
    {true, false, true, true, false, true, false, true, false, true, false,
     true}, // Melodic minor (asc)
    {true, false, true, false, true, false, true, false, true, false, true,
     false}, // Minor pentatonic
    {true, false, true, false, true, true, false, true, false, true, false,
     true}, // Mixolydian
    {true, false, true, false, true, false, true, true, false, true, false,
     true}, // Neapolitan major
    {true, false, true, false, true, false, false, true, false, true, false,
     false}, // Neapolitan minor
    {true, false, true, false, true, true, false, true, false, true, true,
     false}, // Octatonic
    {true, false, true, false, true, false, true, false, true, false, true,
     false}, // Persian
    {true, false, true, true, false, true, false, true, false, true, false,
     true}, // Phrygian dominant
    {true, false, true, false, true, false, true, false, true, false, true,
     false}, // Phrygian
    {true, false, true, true, false, true, false, true, false, true, false,
     true}, // Prometheus
    {true, false, false, true, false, false, true, false, false, true, false,
     false}, // Tritone
    {true, false, true, true, false, false, true, true, false, true, true,
     false}, // Ukranian
    {true, false, true, false, true, false, true, false, true, false, true,
     false}}; // Whole tone

// Return the offset from the root note in semitones for the given scale and
// "scale number", taking into account the scale root
uint8_t getSemitonesOffset(uint8_t scale, uint8_t number, uint8_t root) {

  // assert for valid ranges of scale, number and root
  NAssert(scale < numScales);
  NAssert(number < 12);
  NAssert(root < 12);

  // Find the nth note in the scale (where n is the number parameter)
  uint8_t i = 0;
  uint8_t foundNotes = 0;

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
