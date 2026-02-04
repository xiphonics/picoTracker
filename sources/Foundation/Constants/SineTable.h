/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#pragma once

// 64-step (+ sentinel) sine wave lookup table (full cycle)
// sin(2 * pi * index / 64) * 32767
constexpr int16_t sine_i16_64[65] = {
    0,      3212,   6393,   9512,   12539,  15446,  18204,  20787,  23170,
    25329,  27245,  28898,  30273,  31356,  32137,  32609,  32767,  32609,
    32137,  31356,  30273,  28898,  27245,  25329,  23170,  20787,  18204,
    15446,  12539,  9512,   6393,   3212,   0,      -3212,  -6393,  -9512,
    -12539, -15446, -18204, -20787, -23170, -25329, -27245, -28898, -30273,
    -31356, -32137, -32609, -32767, -32609, -32137, -31356, -30273, -28898,
    -27245, -25329, -23170, -20787, -18204, -15446, -12539, -9512,  -6393,
    -3212,  0,
};

// 16bit interpolation for 64-step sine table
static inline int32_t sine_i16_interpolated_64(uint16_t phase) {
  // index into the table (top 6 bits) and fractional part (lower 10 bits)
  uint8_t index = phase >> 10;   // 0..63
  int32_t frac = phase & 0x03FF; // 0..1023

  // fetch table values
  int32_t a = sine_i16_64[index];
  int32_t b = sine_i16_64[index + 1];

  // a + ((b - a) * frac) / 1024
  return a + (((b - a) * frac) >> 10);
}
