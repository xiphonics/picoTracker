/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "Phrase.h"
#include "Application/Utils/AllocUtils.h"
#include "System/System/System.h"
#include <stdlib.h>
#include <string.h>

Phrase::Phrase() { Reset(); };

Phrase::~Phrase(){};

void Phrase::Reset() {
  for (int i = 0; i < PHRASE_COUNT * STEPS_PER_PHRASE; i++) {
    note_[i] = 0xFF;
    instr_[i] = 0xFF;
    cmd1_[i] = FourCC::InstrumentCommandNone;
    param1_[i] = 0x00;
    cmd2_[i] = FourCC::InstrumentCommandNone;
    param2_[i] = 0x00;
  }
  for (int i = 0; i < PHRASE_COUNT; i++) {
    isUsed_[i] = false;
  }
}

unsigned short Phrase::GetNext() {
  for (int i = 0; i < PHRASE_COUNT; i++) {
    if (!isUsed_[i]) {
      isUsed_[i] = true;
      return i;
    }
  }
  return NO_MORE_PHRASE;
};

unsigned short Phrase::GetNextAfter(uchar current) {
  int next = FindNextAfter(
      current, PHRASE_COUNT, [this](int i) { return !isUsed_[i]; },
      [this](int i) { isUsed_[i] = true; });
  return (next >= 0) ? static_cast<unsigned short>(next) : NO_MORE_PHRASE;
};

void Phrase::SetUsed(unsigned char c) { isUsed_[c] = true; }

void Phrase::ClearAllocation() {

  for (int i = 0; i < PHRASE_COUNT; i++) {
    isUsed_[i] = false;
  }
};
