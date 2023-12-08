#ifndef _AUTONAMES_H_
#define _AUTONAMES_H_

#include <string>
#include "time.h"

const char *adjectives[] = {
    "bad",
    "mad",
    "sad",
    "big",
    "hot",
    "red",
    "wet",
    "low",
    "fat",
    "thin",
    "cold",
    "high",
    "good",
    "sour",
    "sweet",
    "slow",
    "fast",
    "dark",
    "blue",
    "pink",
    "cyan",
    "load",
    "snug",
    "long",
    "hard",
    "soft",
    "mean",
    "lost",
    "busy",
    "last"
};

const char* verbs[] = {
    "sun",
    "sky",
    "car",
    "jet",
    "hut",
    "cat",
    "bat",
    "fox",
    "day",
    "bay",
    "ski",
    "egg",
    "pot",
    "pan",
    "box",
    "pie",
    "cap",
    "tie",
    "fog",
    "map",
    "fig",
    "toy",
    "jug",
    "bug",
    "mug",
    "paw",
    "arm",
    "sea",
    "dog",
    "ray",
    "bag",
    "log",
    "pin",
    "tea",
    "cow",
    "rug",
    "lab",
    "hub",
    "pub",
    "pea",
    "mop",
    "fee",
    "nib",
    "eel",
    "zen",
    "gas",
    "leg",
    "jam",
    "row",
    "air",
    "age",
    "art",
    "hat",
    "lip",
    "ink",
    "pad",
    "toe",
    "axe",
    "nut",
    "bar",
    "ivy",
    "dye",
    "ion",
    "dam",
    "ash",
    "peg",
    "hen",
    "cue",
    "spa",
    "ale",
    "owl",
    "bed",
    "oil",
    "cup",
    "tax",
    "van",
    "bid",
    "gap",
    "cut",
    "tip",
    "ace",
    "gig",
    "web",
    "spy",
    "rye",
    "ark",
    "rag",
    "set",
    "net",
    "bet",
    "bun",
    "pit",
    "era",
    "zoo",
    "tub",
    "gin",
    "app",
    "job",
    "elk",
    "ape",
    "gym"
};

class RandomNames {
  public:
  RandomNames() { srand((unsigned)time(NULL)); }
  std::string getName();
};

// Generate a random name made in the format of: "adjective-verb"
// chosen from small word lists of words 3-4 chars in length
// eg. "fast-sea", "wet-pea", "mad-paw" etc
std::string RandomNames::getName() {
  std::string name = std::string{""};

  int adjectivesCount = sizeof(adjectives) / sizeof(char *);
  int verbsCount = sizeof(verbs) / sizeof(char *);
  name += adjectives[rand() % adjectivesCount];
  name += "-";
  name += verbs[rand() % verbsCount];

  return name;
}

#endif