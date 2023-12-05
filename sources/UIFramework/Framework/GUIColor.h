#ifndef _GUICOLOR_H_
#define _GUICOLOR_H_
#include <stdint.h>

// A simple RGB Color representation class

class GUIColor {
public:
  GUIColor(unsigned short r, unsigned short g, unsigned short b) {
    _r = r;
    _g = g;
    _b = b;
  }
  GUIColor(unsigned short r, unsigned short g, unsigned short b, int idx) {
    _r = r;
    _g = g;
    _b = b;
    _paletteIndex = idx;
  }
  unsigned short _r, _g, _b;
  int _paletteIndex;
};
#endif
