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
#include <cstdint>

#define CHAIN_COUNT 0xFF
#define NO_MORE_CHAIN 0x100
#define PHRASES_PER_CHAIN 0x10

class Chain {
public:
  Chain();
  ~Chain();
  void Reset();
  // Search from start, wrapping to zero when no higher slot is free.
  uint16_t GetNext(uint16_t start = 0);
  bool IsUsed(unsigned char i) { return isUsed_[i]; };
  void SetUsed(unsigned char c);
  void ClearAllocation();

  unsigned char data_[CHAIN_COUNT * PHRASES_PER_CHAIN];
  unsigned char transpose_[CHAIN_COUNT * PHRASES_PER_CHAIN];

private:
  std::bitset<CHAIN_COUNT> isUsed_;
};

#endif
