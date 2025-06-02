#ifndef _PHRASE_H_
#define _PHRASE_H_

#include "Foundation/Types/Types.h"
#define PHRASE_COUNT 0x80
#define NO_MORE_PHRASE 0x81
#define PHRASE_ROW_COUNT 16
#define NO_NOTE_ASSIGNED 0xFF
#define NO_INSTRUMENT_ASSIGNED 0xFF

class Phrase {
public:
  Phrase();
  ~Phrase();
  unsigned short GetNext();
  bool IsUsed(uchar i) { return isUsed_[i]; };
  void SetUsed(uchar c);
  void ClearAllocation();

  uchar note_[PHRASE_COUNT * PHRASE_ROW_COUNT];
  uchar instr_[PHRASE_COUNT * PHRASE_ROW_COUNT];
  FourCC cmd1_[PHRASE_COUNT * PHRASE_ROW_COUNT];
  ushort param1_[PHRASE_COUNT * PHRASE_ROW_COUNT];
  FourCC cmd2_[PHRASE_COUNT * PHRASE_ROW_COUNT];
  ushort param2_[PHRASE_COUNT * PHRASE_ROW_COUNT];

private:
  bool isUsed_[PHRASE_COUNT];
};

#endif
