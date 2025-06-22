/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _CHAIN_H_
#define _CHAIN_H_

#include <bitset>

#define CHAIN_COUNT 0xFF
#define NO_MORE_CHAIN 0x100

class Chain {
public:
  Chain();
  ~Chain();
  unsigned short GetNext();
  bool IsUsed(unsigned char i) { return isUsed_[i]; };
  void SetUsed(unsigned char c);
  void ClearAllocation();

  unsigned char data_[CHAIN_COUNT * 16];
  unsigned char transpose_[CHAIN_COUNT * 16];

private:
  std::bitset<CHAIN_COUNT> isUsed_;
};

#endif
