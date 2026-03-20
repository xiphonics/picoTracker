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
  const char states[6] = {
      GLYPH(char_propgress_bar_0_s), GLYPH(char_propgress_bar_1_s),
      GLYPH(char_propgress_bar_2_s), GLYPH(char_propgress_bar_3_s),
      GLYPH(char_propgress_bar_4_s), GLYPH(char_block_full_s)};

  (*progressBar)[0] = GLYPH(char_button_border_left_s);
  (*progressBar)[11] = GLYPH(char_button_border_right_s);
  (*progressBar)[12] = 0;

  int32_t prog60 =
      (max == 0U) ? 60 : static_cast<int32_t>((progress * 60U) / max);

  for (int32_t j = 1; j < 11; j++) {
    int32_t val = std::clamp(prog60, int32_t{0}, int32_t{5});
    (*progressBar)[j] = states[val];
    prog60 -= 6;
  }
}
