#ifndef SCALE_VIEW_H
#define SCALE_VIEW_H

const int numScales = 44;
extern const char *scaleNames[numScales];
extern const bool scaleSteps[numScales][12];
extern const char *noteNames[12];

// Function that calculates semitone offset based on scale, position in scale,
// and root note
unsigned char getSemitonesOffset(unsigned char scale, unsigned char number,
                                 unsigned char root = 0);

#endif
