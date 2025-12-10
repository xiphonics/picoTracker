/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "Chain.h"
#include "System/System/System.h"
#include <stdlib.h>
#include <string.h>

Chain::Chain() {

  for (int i = 0; i < CHAIN_COUNT * PHRASES_PER_CHAIN; i++) {
    data_[i] = 0xFF;
    transpose_[i] = 0x00;
  }
  for (int i = 0; i < CHAIN_COUNT; i++) {
    isUsed_[i] = false;
  }
};

Chain::~Chain(){};

unsigned short Chain::GetNext() {
  for (int i = 0; i < CHAIN_COUNT; i++) {
    if (!isUsed_[i]) {
      isUsed_[i] = true;
      return i;
    }
  }
  return NO_MORE_CHAIN;
};

void Chain::SetUsed(unsigned char c) { isUsed_[c] = true; }

void Chain::ClearAllocation() {

  for (int i = 0; i < CHAIN_COUNT; i++) {
    isUsed_[i] = false;
  }
}
