/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _CHAR_H_
#define _CHAR_H_

#include "Foundation/Types/Types.h"
#include <string.h>

extern char h2c__[16];
extern const char *noteNames[12];

inline void hex2char(const unsigned char c, char *s) {
  char *dest__ = s;
  *dest__++ = h2c__[(c & 0xF0) >> 4];
  *dest__++ = h2c__[c & 0x0F];
  *dest__ = 0;
}

inline void hexshort2char(const ushort c, char *s) {
  char *dest__ = s;
  *dest__++ = h2c__[(c & 0xF000) >> 12];
  *dest__++ = h2c__[(c & 0xF00) >> 8];
  *dest__++ = h2c__[(c & 0xF0) >> 4];
  *dest__++ = h2c__[c & 0x0F];
  *dest__ = 0;
}

#define c2h__(c) (c < 'A' ? c - '0' : c - 'A' + 10)

inline void char2hex(const char *s, unsigned char *c) {
  const char *src__ = s;
  unsigned char b1 = (c2h__(src__[0])) << 4;
  unsigned char b2 = c2h__(src__[1]);
  *c = b1 + b2;
}

inline void note2char(unsigned char d, char *s) {
  int oct = d / 12 - 2;
  int note = d % 12;
  strcpy(s, noteNames[note]);
  if (oct < 0) {
    s[2] = '-';
    oct = -oct;
  } else {
    s[2] = ' ';
  }
  s[3] = '0' + oct;
};

inline void note2visualizer(unsigned char d, char *s) {
  int note = d % 12;
  strcpy(s, noteNames[note]);
  s[2] = '\0'; // sloppy, can we make the array shorter?
  s[3] = '\0'; // sloppy, can we make the array shorter?
};

inline void oct2visualizer(unsigned char d, char *s) {
  int oct = d / 12 - 2;
  if (oct < 0) {
    s[0] = '-';
    oct = -oct;
  } else {
    s[0] = ' ';
  }
  s[1] = '0' + oct;
  s[2] = '\0'; // sloppy, can we make the array shorter?
  s[3] = '\0'; // sloppy, can we make the array shorter?
};

#endif
