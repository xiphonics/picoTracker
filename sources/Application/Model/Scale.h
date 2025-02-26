#ifndef SCALE_VIEW_H
#define SCALE_VIEW_H

const int numScales = 44;
extern const char *scaleNames[numScales];
extern const bool scaleSteps[numScales][12];
unsigned char getSemitonesOffset(unsigned char scale, unsigned char number);

#endif
