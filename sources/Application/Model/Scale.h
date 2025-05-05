#ifndef SCALE_VIEW_H
#define SCALE_VIEW_H

#include <cstdint>

const int numScales = 44;
extern const char *scaleNames[numScales];
extern const bool scaleSteps[numScales][12];
extern const char *noteNames[12];

// Function that calculates semitone offset based on scale, position in scale,
// and root note
unsigned char getSemitonesOffset(uint8_t scale, uint8_t number, uint8_t root);

#endif
