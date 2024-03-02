#include "Phrase.h"
#include "System/System/System.h"
#include <stdlib.h>
#include <string.h>

Phrase::Phrase() {
  for (int i = 0; i < PHRASE_COUNT * 16; i++) {
    note_[i] = 0xFF;
    instr_[i] = 0xFF;
    cmd1_[i] = MAKE_FOURCC('-', '-', '-', '-');
    param1_[i] = 0x00;
    cmd2_[i] = MAKE_FOURCC('-', '-', '-', '-');
    param2_[i] = 0x00;
  }
  for (int i = 0; i < PHRASE_COUNT; i++) {
    isUsed_[i] = false;
  }
};

Phrase::~Phrase(){};

unsigned short Phrase::GetNext() {
  for (int i = 0; i < PHRASE_COUNT; i++) {
    if (!isUsed_[i]) {
      isUsed_[i] = true;
      return i;
    }
  }
  return NO_MORE_PHRASE;
};

void Phrase::SetUsed(unsigned char c) { isUsed_[c] = true; }

void Phrase::ClearAllocation() {

  for (int i = 0; i < PHRASE_COUNT; i++) {
    isUsed_[i] = false;
  }
};
