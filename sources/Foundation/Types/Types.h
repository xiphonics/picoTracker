#ifndef _APP_TYPES_H_
#define _APP_TYPES_H_

#include <stdint.h>

// #define FOURCC(i) (((i&0xff000000)>>24) | ((i&0x00ff0000)>>8) |
// ((i&0x0000ff00)<<8) | ((i&0x000000ff)<<24)) #ifdef __ppc__ #define
// MAKE_FOURCC(ch0,ch1,ch2,ch3) (ch3 | ch2<<8 | ch1<<16 | ch0<<24) #else #define
// MAKE_FOURCC(ch0,ch1,ch2,ch3) (ch0 | ch1<<8 | ch2<<16 | ch3<<24) #endif

#ifdef WIN32
#define strcasecmp _stricmp
#endif

// typedef unsigned int FourCC ;
typedef unsigned char FourCC;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned char uchar;

// TODO: this is a lazy drop in replacement (alongside changes in
// Application/Utils/char.h) for the FourCC type using 1 byte instead of 4
// There's probably a better way to do this with more refactoring
inline constexpr FourCC MAKE_FOURCC(char ch0, char ch1, char ch2, char ch3) {
  uint32_t fourcc = ch0 | ch1 << 8 | ch2 << 16 | ch3 << 24;
  switch (fourcc) {
  case 'A' | 'R' << 8 | 'P' << 16 | 'G' << 24:
    return 0;
    break;
  case 'C' | 'H' << 8 | 'N' << 16 | 'L' << 24:
    return 1;
    break;
  case 'C' | 'R' << 8 | 'S' << 16 | 'H' << 24:
    return 2;
    break;
  case 'C' | 'R' << 8 | 'S' << 16 | 'V' << 24:
    return 3;
    break;
  case 'D' | 'L' << 8 | 'A' << 16 | 'Y' << 24:
    return 4;
    break;
  case 'D' | 'S' << 8 | 'P' << 16 | 'L' << 24:
    return 5;
    break;
  case 'E' | 'N' << 8 | 'D' << 16 | '_' << 24:
    return 6;
    break;
  case 'E' | 'V' << 8 | 'A' << 16 | '_' << 24:
    return 7;
    break;
  case 'E' | 'V' << 8 | 'B' << 16 | '_' << 24:
    return 8;
    break;
  case 'E' | 'V' << 8 | 'D' << 16 | 'N' << 24:
    return 9;
    break;
  case 'E' | 'V' << 8 | 'L' << 16 | 'F' << 24:
    return 10;
    break;
  case 'E' | 'V' << 8 | 'L' << 16 | 'S' << 24:
    return 11;
    break;
  case 'E' | 'V' << 8 | 'R' << 16 | 'S' << 24:
    return 12;
    break;
  case 'E' | 'V' << 8 | 'R' << 16 | 'T' << 24:
    return 13;
    break;
  case 'E' | 'V' << 8 | 'S' << 16 | 'T' << 24:
    return 14;
    break;
  case 'E' | 'V' << 8 | 'U' << 16 | 'P' << 24:
    return 15;
    break;
  case 'F' | 'B' << 8 | 'A' << 16 | 'M' << 24:
    return 16;
    break;
  case 'F' | 'B' << 8 | 'T' << 16 | 'U' << 24:
    return 19;
    break;
  case 'F' | 'C' << 8 | 'U' << 16 | 'T' << 24:
    return 20;
    break;
  case 'F' | 'I' << 8 | 'M' << 16 | 'O' << 24:
    return 21;
    break;
  case 'F' | 'L' << 8 | 'T' << 16 | 'R' << 24:
    return 22;
    break;
  case 'F' | 'M' << 8 | 'I' << 16 | 'X' << 24:
    return 23;
    break;
  case 'F' | 'N' << 8 | 'T' << 16 | 'N' << 24:
    return 24;
    break;
  case 'F' | 'R' << 8 | 'E' << 16 | 'S' << 24:
    return 25;
    break;
  case 'G' | 'R' << 8 | 'O' << 16 | 'V' << 24:
    return 26;
    break;
  case 'H' | 'O' << 8 | 'P' << 16 | ' ' << 24:
    return 27;
    break;
  case 'I' | 'N' << 8 | 'T' << 16 | 'P' << 24:
    return 28;
    break;
  case 'I' | 'R' << 8 | 'T' << 16 | 'G' << 24:
    return 29;
    break;
  case 'K' | 'I' << 8 | 'L' << 16 | 'L' << 24:
    return 30;
    break;
  case 'L' | 'E' << 8 | 'G' << 16 | 'A' << 24:
    return 31;
    break;
  case 'L' | 'E' << 8 | 'N' << 16 | 'G' << 24:
    return 32;
    break;
  case 'L' | 'L' << 8 | 'E' << 16 | 'N' << 24:
    return 33;
    break;
  case 'L' | 'M' << 8 | 'O' << 16 | 'D' << 24:
    return 34;
    break;
  case 'L' | 'O' << 8 | 'A' << 16 | 'D' << 24:
    return 35;
    break;
  case 'L' | 'P' << 8 | 'O' << 16 | 'F' << 24:
    return 36;
    break;
  case 'L' | 'S' << 8 | 'T' << 16 | 'A' << 24:
    return 37;
    break;
  case 'M' | 'D' << 8 | 'C' << 16 | 'C' << 24:
    return 38;
    break;
  case 'M' | 'D' << 8 | 'P' << 16 | 'G' << 24:
    return 39;
    break;
  case 'M' | 'I' << 8 | 'D' << 16 | 'I' << 24:
    return 40;
    break;
  case 'M' | 'S' << 8 | 'T' << 16 | 'R' << 24:
    return 41;
    break;
  case 'P' | 'A' << 8 | 'N' << 16 | ' ' << 24:
    return 42;
    break;
  case 'P' | 'A' << 8 | 'N' << 16 | '_' << 24:
    return 43;
    break;
  case 'P' | 'F' << 8 | 'I' << 16 | 'N' << 24:
    return 44;
    break;
  case '-' | '-' << 8 | '-' << 16 | '-' << 24:
    // arbitrary, original char to be used for "blank" is "-" (0x2D)
    return 45;
    break;
  case 'P' | 'L' << 8 | 'O' << 16 | 'F' << 24:
    return 46;
    break;
  case 'P' | 'R' << 8 | 'G' << 16 | 'I' << 24:
    return 47;
    break;
  case 'P' | 'T' << 8 | 'C' << 16 | 'H' << 24:
    return 48;
    break;
  case 'P' | 'U' << 8 | 'R' << 16 | 'G' << 24:
    return 49;
    break;
  case 'R' | 'O' << 8 | 'O' << 16 | 'T' << 24:
    return 51;
    break;
  case 'R' | 'T' << 8 | 'R' << 16 | 'G' << 24:
    return 52;
    break;
  case 'S' | 'A' << 8 | 'V' << 16 | 'E' << 24:
    return 53;
    break;
  case 'S' | 'M' << 8 | 'P' << 16 | 'L' << 24:
    return 54;
    break;
  case 'S' | 'T' << 8 | 'O' << 16 | 'P' << 24:
    return 55;
    break;
  case 'S' | 'T' << 8 | 'R' << 16 | 'T' << 24:
    return 56;
    break;
  case 'S' | 'V' << 8 | 'P' << 16 | 'S' << 24:
    return 57;
    break;
  case 'T' | 'A' << 8 | 'B' << 16 | 'L' << 24:
    return 58;
    break;
  case 'T' | 'B' << 8 | 'E' << 16 | 'D' << 24:
    return 59;
    break;
  case 'T' | 'B' << 8 | 'L' << 16 | 'A' << 24:
    return 60;
    break;
  case 'T' | 'E' << 8 | 'M' << 16 | 'P' << 24:
    return 61;
    break;
  case 'T' | 'M' << 8 | 'P' << 16 | 'O' << 24:
    return 62;
    break;
  case 'T' | 'R' << 8 | 'S' << 16 | 'P' << 24:
    return 63;
    break;
  case 'T' | 'S' << 8 | 'Q' << 16 | 'R' << 24:
    return 64;
    break;
  case 'T' | 'T' << 8 | 'A' << 16 | 'P' << 24:
    return 65;
    break;
  case 'T' | 'X' << 8 | 'H' << 16 | '1' << 24:
    return 71;
    break;
  case 'T' | 'X' << 8 | 'V' << 16 | '1' << 24:
    return 72;
    break;
  case 'V' | 'E' << 8 | 'L' << 16 | 'M' << 24:
    return 66;
    break;
  case 'V' | 'L' << 8 | 'D' << 16 | 'N' << 24:
    return 67;
    break;
  case 'V' | 'L' << 8 | 'U' << 16 | 'P' << 24:
    return 68;
    break;
  case 'V' | 'O' << 8 | 'L' << 16 | 'M' << 24:
    return 69;
    break;
  case 'W' | 'R' << 8 | 'A' << 16 | 'P' << 24:
    return 70;
    break;
  default:
    return 255;
  }
}

#endif
