/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "Foundation/Constants/SpecialCharacters.h"
#include <cstdint>

typedef char progressBar_t[13];

static void fillProgressBar(uint32_t progress, uint32_t max,
                            progressBar_t *progressBar) {
  const char states[6] = {char_propgress_bar_0, char_propgress_bar_1,
                          char_propgress_bar_2, char_propgress_bar_3,
                          char_propgress_bar_4, char_block_full};

  (*progressBar)[0] = char_button_border_left;
  (*progressBar)[11] = char_button_border_right;
  (*progressBar)[12] = 0;

  int32_t prog60 =
      (max == 0U) ? 60 : static_cast<int32_t>((progress * 60U) / max);

  for (int32_t j = 1; j < 11; j++) {
    int32_t val = std::clamp(prog60, int32_t{0}, int32_t{5});
    (*progressBar)[j] = states[val];
    prog60 -= 6;
  }
}
