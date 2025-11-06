/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _PHRASE_H_
#define _PHRASE_H_

#include "Foundation/Types/Types.h"
#ifdef ADV
#define PHRASE_COUNT 0xFF
#define NO_MORE_PHRASE 0x100
#else
#define PHRASE_COUNT 0x80
#define NO_MORE_PHRASE 0x81
#endif

class Phrase {
public:
  Phrase();
  ~Phrase();
  unsigned short GetNext();
  bool IsUsed(uchar i) { return isUsed_[i]; };
  void SetUsed(uchar c);
  void ClearAllocation();

  uchar note_[PHRASE_COUNT * 16];
  uchar instr_[PHRASE_COUNT * 16];
  FourCC cmd1_[PHRASE_COUNT * 16];
  ushort param1_[PHRASE_COUNT * 16];
  FourCC cmd2_[PHRASE_COUNT * 16];
  ushort param2_[PHRASE_COUNT * 16];

private:
  bool isUsed_[PHRASE_COUNT];
};

#endif
