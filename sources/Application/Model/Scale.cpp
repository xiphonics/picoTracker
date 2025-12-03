/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

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
    // 0     1     2     3     4     5     6     7     8     9    10    11
    {true, true, true, true, true, true, true, true, true, true, true, true}, // None (Chromatic)
    // 0            2           4            6      7            9
    {true, false, true, false, true, false, true, true, false, true, false, false}, // Acoustic
    // 0            2           4     5             7    8            10
    {true, false, true, false, true, true, false, true, true, false, true, false}, // Adonal malakh
    // 0           2      3           5             7    8            10
    {true, false, true, true, false, true, false, true, true, false, true, false}, // Aeolian mode (minor)
    // 0           2      3                  6      7    8                   11
    {true, false, true, true, false, false, true, true, true, false, false, true}, // Algerian
    // 0     1            3     4           6            8            10
    {true, true, false, true, true, false, true, false, true, false, true, false}, // Altered
    // 0                  3     4                   7     8                    11
    {true, false, false, true, true, false, false, true, true, false, false, true}, // Augmented
    // 0            2           4      5            7            9     10    11
    {true, false, true, false, true, true, false, true, false, true, true, true}, // Bebop dominant
    // 0                   3           5     6      7                  10
    {true, false, false, true, false, true, true, true, false, false, true, false}, // Blues
    // 0            2     3            5            7           9     10
    {true, false, true, true, false, true, false, true, false, true, true, false}, // Dorian
    // 0    1                    4     5            7    8                   11
    {true, true, false, false, true, true, false, true, true, false, false, true}, // Double harmonic
    // 0    1                    4           6            8            10    11
    {true, true, false, false, true, false, true, false, true, false, true, true}, // Enigmatic
    // 0    1                    4    5             7     8                  11
    {true, true, false, false, true, true, false, true, true, false, false, true}, // Flamenco
    // 0           2      3                   6     7     8           10
    {true, false, true, true, false, false, true, true, true, false, true, false}, // Gypsy
    // 0           2      3           5      6           8            10
    {true, false, true, true, false, true, true, false, true, false, true, false}, // Half diminished
    // 0           2            4      5            7     8                  11
    {true, false, true, false, true, true, false, true, true, false, false, true}, // Harmonic major
    // 0           2     3             5           7      8                  11
    {true, false, true, true, false, true, false, true, true, false, false, true}, // Harmonic minor
    // 0           2     3                          7     8
    {true, false, true, true, false, false, false, true, true, false, false, false}, // Hirajoshi
    // 0            2    3                   6     7      8                  11
    {true, false, true, true, false, false, true, true, true, false, false, true}, // Hungarian gypsy
    // 0            2    3                   6     7      8                  11
    {true, false, true, true, false, false, true, true, true, false, false, true}, // Hungarian minor
    // 0    1                           5           7                   10
    {true, true, false, false, false, true, false, true, false, false, true, false}, // Insen
    // 0            2            4     5            7           9            11
    {true, false, true, false, true, true, false, true, false, true, false, true}, // Ionian mode (major)
    // 0     1            3     4           6      7
    {true, true, false, true, true, false, true, true, false, false, false, false}, // Istrian
    // 0    1                          5      6                         10
    {true, true, false, false, false, true, true, false, false, false, true, false}, // Iwato
    // 0    1            3             5     6      7                 10
    {true, true, false, true, false, true, true, false, true, false, true, false}, // Locrian
    // 0           2            4            6            8      9           11
    {true, false, true, false, true, false, true, false, true, true, false, true}, // Lydian augmented
    // 0           2            4            6      7            9           11
    {true, false, true, false, true, false, true, true, false, true, false, true}, // Lydian
    // 0           2            4     5             7    8     9            11
    {true, false, true, false, true, true, false, true, true, true, false, true}, // Major bebop
    // 0           2            4     5      6           8            10
    {true, false, true, false, true, true, true, false, true, false, true, false}, // Major locrian
    // 0           2            4                   7            9
    {true, false, true, false, true, false, false, true, false, true, false, false}, // Major pentatonic
    // 0           2      3            5           7     8      9    10    11
    {true, false, true, true, false, true, false, true, true, true, true, true}, // Melodic minor
    // 0           2      3            5           7            9            11
    {true, false, true, true, false, true, false, true, false, true, false, true}, // Melodic minor (asc)
    // 0                  3            5           7                   10
    {true, false, false, true, false, true, false, true, false, false, true, false}, // Minor pentatonic
    // 0            2           4      5           7            9     10
    {true, false, true, false, true, true, false, true, false, true, true, false}, // Mixolydian
    // 0    1            3            5            7            9            11
    {true, true, false, true, false, true, false, true, false, true, false, true}, // Neapolitan major
    // 0    1            3            5            7      8                  11
    {true, true, false, true, false, true, false, true, true, false, false, true}, // Neapolitan minor
    // 0           2     3            5      6            8     9           11
    {true, false, true, true, false, true, true, false, true, true, false, true}, // Octatonic
    // 0    1                   4     5     6            8                   11
    {true, true, false, false, true, true, true, false, true, false, false, true}, // Persian
    // 0    1                   4     5            7     8            10
    {true, true, false, false, true, true, false, true, true, false, true, false}, // Phrygian dominant
    // 0    1            3            5            7     8            10
    {true, true, false, true, false, true, false, true, true, false, true, false}, // Phrygian
    // 0           2            4            6                    9    10
    {true, false, true, false, true, false, true, false, false, true, true, false}, // Prometheus
    // 0    1                   4            6     7                   10
    {true, true, false, false, true, false, true, true, false, false, true, false}, // Tritone
    // 0           2     3                   6     7            9     10
    {true, false, true, true, false, false, true, true, false, true, true, false}, // Ukranian
    // 0           2            4            6            8            10
    {true, false, true, false, true, false, true, false, true, false, true, false}}; // Whole tone

// Return the offset from the root note in semitones for the given scale and
// "scale number", taking into account the scale root
uint8_t getSemitonesOffset(uint8_t scale, uint8_t number, uint8_t root) {

  // check for valid ranges of scale, number and root
  if (scale >= numScales || number >= 12 || root >= 12) {
    NAssert(false);
    return 0;
  }

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
