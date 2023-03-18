
#ifndef _TEXT_MODE_H
#define _TEXT_MODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ili9341.h"

// ARNE-16 palette converted to RGB565 --
// https://lospec.com/palette-list/arne-16
typedef enum {
  MODE0_BG,
  MODE0_NORMAL,
  MODE0_HILITE,
  MODE0_HILITE2,
  MODE0_GRAY,
  MODE0_DESERT,
  MODE0_ORANGE,
  MODE0_YELLOW,
  MODE0_WHITE,
  MODE0_MIDNIGHT,
  MODE0_DARK_SLATE_GRAY,
  MODE0_GREEN,
  MODE0_YELLOW_GREEN,
  MODE0_BLUE,
  MODE0_PICTON_BLUE,
  MODE0_PALE_BLUE
} mode0_color_t;

void mode0_init();
void mode0_clear(mode0_color_t color);
void mode0_draw_screen();
void mode0_draw_changed();
void mode0_draw_changed_simple();
void mode0_draw_sub_region(uint8_t x, uint8_t y, uint8_t width, uint8_t height);
void mode0_draw_region(uint8_t x, uint8_t y, uint8_t width, uint8_t height);
void mode0_set_foreground(mode0_color_t color);
void mode0_set_background(mode0_color_t color);
void mode0_set_cursor(uint8_t x, uint8_t y);
uint8_t mode0_get_cursor_x();
uint8_t mode0_get_cursor_y();
void mode0_print(const char *s, bool invert);
void mode0_write(const char *s, int len, bool invert);
void mode0_putc(char c, bool invert);
void mode0_set_palette_color(int idx, uint16_t rgb565_color);

#ifdef __cplusplus
}
#endif
#endif
