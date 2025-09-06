/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _TEXT_CHARGFX_H
#define _TEXT_CHARGFX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ili9341.h"

#define TEXT_WIDTH 32
#define TEXT_HEIGHT 24
#define CHAR_HEIGHT 10
#define CHAR_WIDTH 10
#define BUFFER_CHARS 12

// ARNE-16 palette converted to RGB565 --
// https://lospec.com/palette-list/arne-16
typedef enum {
  CHARGFX_BG,
  CHARGFX_NORMAL,
  CHARGFX_HILITE,
  CHARGFX_HILITE2,
  CHARGFX_GRAY,
  CHARGFX_DESERT,
  CHARGFX_ORANGE,
  CHARGFX_YELLOW,
  CHARGFX_WHITE,
  CHARGFX_MIDNIGHT,
  CHARGFX_DARK_SLATE_GRAY,
  CHARGFX_GREEN,
  CHARGFX_YELLOW_GREEN,
  CHARGFX_BLUE,
  CHARGFX_GURU_TXT,
  CHARGFX_GURU_BG
} chargfx_color_t;

void chargfx_init();
void chargfx_clear(chargfx_color_t color);
void chargfx_draw_screen();
void chargfx_draw_changed();
void chargfx_draw_changed_simple();
void chargfx_draw_sub_region(uint8_t x, uint8_t y, uint8_t width,
                             uint8_t height);
void chargfx_draw_region(uint8_t x, uint8_t y, uint8_t width, uint8_t height);
void chargfx_fill_rect(uint8_t color_index, uint16_t x, uint16_t y,
                       uint16_t width, uint16_t height);
void chargfx_set_foreground(chargfx_color_t color);
void chargfx_set_background(chargfx_color_t color);
void chargfx_set_cursor(uint8_t x, uint8_t y);
uint8_t chargfx_get_cursor_x();
uint8_t chargfx_get_cursor_y();
void chargfx_print(const char *s, bool invert);
void chargfx_write(const char *s, int len, bool invert);
void chargfx_putc(char c, bool invert);
void chargfx_set_palette_color(int idx, uint16_t rgb565_color);
void chargfx_set_font_index(uint8_t idx);

#ifdef __cplusplus
}
#endif
#endif
