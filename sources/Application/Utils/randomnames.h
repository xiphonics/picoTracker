/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _RANDOMNAMES_H_
#define _RANDOMNAMES_H_

#include <string.h>

const char *adjectives[] = {"bad",  "mad",  "sad",   "big",  "hot",  "red",
                            "wet",  "low",  "fat",   "thin", "cold", "high",
                            "good", "sour", "sweet", "slow", "fast", "dark",
                            "blue", "pink", "cyan",  "load", "snug", "long",
                            "hard", "soft", "mean",  "lost", "busy", "last"};

const char *verbs[] = {
    "sun", "sky", "car", "jet", "hut", "cat", "bat", "fox", "day", "bay", "ski",
    "egg", "pot", "pan", "box", "pie", "cap", "tie", "fog", "map", "fig", "toy",
    "jug", "bug", "mug", "paw", "arm", "sea", "dog", "ray", "bag", "log", "pin",
    "tea", "cow", "rug", "lab", "hub", "pub", "pea", "mop", "fee", "nib", "eel",
    "zen", "gas", "leg", "jam", "row", "air", "age", "art", "hat", "lip", "ink",
    "pad", "toe", "axe", "nut", "bar", "ivy", "dye", "ion", "dam", "ash", "peg",
    "hen", "cue", "spa", "ale", "owl", "bed", "oil", "cup", "tax", "van", "bid",
    "gap", "cut", "tip", "ace", "gig", "web", "spy", "rye", "ark", "rag", "set",
    "net", "bet", "bun", "pit", "era", "zoo", "tub", "gin", "app", "job", "elk",
    "ape", "gym"};

// Generate a random name made in the format of: "adjective-verb"
// chosen from small word lists of words 3-4 chars in length
// eg. "fast-sea", "wet-pea", "mad-paw" etc
void getRandomName(char *name, uint32_t randomNumber, size_t nameSize) {

  if (!name || nameSize < 10)
    return; // Basic validation

  int adjectivesCount = sizeof(adjectives) / sizeof(adjectives[0]);
  int verbsCount = sizeof(verbs) / sizeof(verbs[0]);
  int rndIndex = randomNumber % adjectivesCount;
  strcpy(name, adjectives[rndIndex]);
  strcat(name, "-");
  rndIndex = randomNumber % verbsCount;
  strcat(name, verbs[rndIndex]);
}

#endif
