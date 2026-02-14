/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "Foundation/Constants/SpecialCharacters.h"

typedef char progressBar_t[13];

static void fillProgressBar(int progress, int max, progressBar_t *progressBar) {
  const char states[6] = {char_battery_empty,   char_propgress_bar_1,
                          char_propgress_bar_2, char_propgress_bar_3,
                          char_propgress_bar_4, char_block_full};

  (*progressBar)[0] = char_button_border_left;
  (*progressBar)[11] = char_button_border_right;
  (*progressBar)[12] = 0;

  int prog60 = (max <= 0) ? 60 : (progress * 60) / max;

  for (int j = 1; j < 11; j++) {
    int val = std::clamp(prog60, 0, 5);
    (*progressBar)[j] = states[val];
    prog60 -= 6;
  }
}