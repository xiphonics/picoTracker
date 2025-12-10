/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#include "display.h"
#include "dma2d.h"
#include "font.h"
#include "usart.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static color_t screen_bg_color = COLOR_BG;
static color_t screen_fg_color = COLOR_NORMAL;
static int cursor_x = 0;
static int cursor_y = 0;
static uint8_t screen[TEXT_HEIGHT * TEXT_WIDTH] = {0};
static uint8_t colors[TEXT_HEIGHT * TEXT_WIDTH] = {0};

static uint8_t ui_font_index = 0;

// Using a bit array in order to save memory, there is a slight performance
// hit in doing so vs a bool array
static uint8_t changed[TEXT_HEIGHT * TEXT_WIDTH / 8] = {0};
#define SetBit(A, k) (A[(k) / 8] |= (1 << ((k) % 8)))
#define ClearBit(A, k) (A[(k) / 8] &= ~(1 << ((k) % 8)))
#define TestBit(A, k) (A[(k) / 8] & (1 << ((k) % 8)))

uint32_t rgb565_to_abgr8888(uint16_t rgb565) {
  uint8_t r = ((rgb565 >> 11) & 0x1F) << 3;
  uint8_t g = ((rgb565 >> 5) & 0x3F) << 2;
  uint8_t b = (rgb565 & 0x1F) << 3;
  return 0xFF000000 | (b << 16) | (g << 8) | r;
}

// Default palette, can be redefined
// Colors are defined as RGB565 in pT (for the moment) but we use ABGR8888 on
// the display driver
static uint32_t palette[16] = {
    0xFF000000, // 0x0000
    0xFF65CC92, // 0x49E5
    0xFF2E92B2, // 0xB926
    0xFF71C5C7, // 0xE371
    0xFF73F19C, // 0x9CF3
    0xFF2494A3, // 0xA324
    0xFF46C3EC, // 0xEC46
    0xFF0DF7F7, // 0xF70D
    0xFFFFFFFF, // 0xFFFF
    0xFF269019, // 0x1926
    0xFF49A12A, // 0x2A49
    0xFF438244, // 0x4443
    0xFF6496A6, // 0xA664
    0xFFB00200, // 0x02B0
    0xFF1E9635, // 0x351E
    0xFFFD6DB6, // 0xB6FD
};

static inline uint16_t abgr8888_to_bgr565(uint32_t color) {
  uint16_t b = (uint16_t)((color >> 19) & 0x1F);
  uint16_t g = (uint16_t)((color >> 10) & 0x3F);
  uint16_t r = (uint16_t)((color >> 3) & 0x1F);
  return (uint16_t)((b << 11) | (g << 5) | r);
}
void display_set_font_index(uint8_t idx) { ui_font_index = idx; }

void display_set_cursor(uint8_t x, uint8_t y) {
  cursor_x = x;
  cursor_y = y;
}

void display_putc(char c, bool invert) {
  int idx = cursor_y * TEXT_WIDTH + cursor_x;
  if (c >= 32 && c <= 127) {
    screen[idx] = c - 32;
    SetBit(changed, idx);
    if (invert) {
      colors[idx] = ((screen_bg_color & 0xf) << 4) | (screen_fg_color & 0xf);
    } else {
      colors[idx] = ((screen_fg_color & 0xf) << 4) | (screen_bg_color & 0xf);
    }
  }
}

void display_print(const char *str, bool invert) {
  char c;
  while ((c = *str++)) {
    display_putc(c, invert);
  }
}

void display_set_foreground(color_t color) { screen_fg_color = color; }

void display_set_background(color_t color) { screen_bg_color = color; }

void display_clear(color_t color) {
  int size = TEXT_WIDTH * TEXT_HEIGHT;
  memset(screen, 0, size);
  memset(colors, color, size);
  display_set_cursor(0, 0);
  display_draw_screen();
}

void display_set_palette_color(int idx, uint16_t rgb565_color) {
  palette[idx] = rgb565_to_abgr8888(rgb565_color);
}

// Draw changed detects subregions of the display that changed and that are of
// same parameters (BG and FG colors)
void display_draw_changed() {
  uint32_t bg_color;
  uint32_t fg_color;
  for (int idx = 0; idx < TEXT_HEIGHT * TEXT_WIDTH; idx++) {
    if (TestBit(changed, idx)) {
      // get foreground and background colors of the area
      fg_color = palette[colors[idx] >> 4];
      bg_color = palette[colors[idx] & 0xf];

      ClearBit(changed, idx);
      // check adjacent in order to find bigger rectangle
      uint16_t y = idx / TEXT_WIDTH;
      uint16_t x = idx - (TEXT_WIDTH * y);

      int height = 1;
      // first pass tests the height
      for (int probe_y = y + 1; probe_y < TEXT_HEIGHT; probe_y++) {
        int probe_idx = probe_y * TEXT_WIDTH + x;
        if (TestBit(changed, probe_idx) &&
            fg_color == palette[colors[probe_idx] >> 4] &&
            bg_color == palette[colors[probe_idx] & 0xf]) {
          ClearBit(changed, probe_idx);
          height++;
          continue;
        }
        break;
      }

      int16_t width = 1;
      // having the height, we can test every subsequent column
      for (int probe_x = x + 1; probe_x < TEXT_WIDTH; probe_x++) {
        for (int probe_y = y; probe_y < y + height; probe_y++) {
          // if we don't get to max height, then abort
          int probe_idx = probe_y * TEXT_WIDTH + probe_x;
          if (!TestBit(changed, probe_idx) ||
              fg_color != palette[colors[probe_idx] >> 4] ||
              bg_color != palette[colors[probe_idx] & 0xf]) {
            // undo last column
            for (int undo_y = y; undo_y < probe_y; undo_y++) {
              SetBit(changed, undo_y * TEXT_WIDTH + probe_x);
            }
            goto end;
          }
          ClearBit(changed, probe_idx);
        }
        width++;
      }
    end:
      display_draw_region(x, y, width, height);
    }
  }
}

void display_draw_screen() {
  // draw the whole screen
  display_draw_region(0, 0, TEXT_WIDTH, TEXT_HEIGHT);
}

void display_draw_region(uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
  // Each region share the same FG and BG color, so we defined at the start and
  // reconfigure the FG layer for the given text color
  uint32_t idx = y * TEXT_WIDTH + x;
  uint32_t bg_color = palette[colors[idx] & 0xF];
  uint32_t fg_color = palette[colors[idx] >> 4];

  hdma2d.LayerCfg[1].InputAlpha = fg_color;
  HAL_DMA2D_ConfigLayer(&hdma2d, 1);

  // Draw each character
  for (uint8_t h = y; h < y + height; h++) {
    for (uint8_t w = x; w < x + width; w++) {

      uint8_t char_index = screen[h * TEXT_WIDTH + w];
      const uint8_t *glyph = &FONT[char_index][0][0];

      uint16_t *dest = (uint16_t *)framebuffer +
                       (h * CHAR_HEIGHT * DISPLAY_WIDTH) +
                       (w * CHAR_WIDTH + OFFSET);

      /* Most of what happens here is defined on the peripheral configuration.
       * Perpheral is configured in memory to memory mode with blending and
       * fixed background color. We take the FG color from the layer config,
       * which we have to update to each region. BG color is defined as a static
       * address with the color in it, so this works as a register to memory
       * DMA2D mode. Char array is contiguous in memory for a single char, but
       * frame buffer needs to have a stride if DISPLAY_WIDTH - CHAR_WIDTH
       * for each line rendered, this is also configured in the peripheral.
       */
      HAL_DMA2D_BlendingStart(&hdma2d, (uint32_t)glyph, bg_color,
                              (uint32_t)dest, CHAR_WIDTH, CHAR_HEIGHT);
      // We could check for return status here, but if it's not returning HAL_OK
      // here we have big problems
      HAL_DMA2D_PollForTransfer(&hdma2d, HAL_MAX_DELAY);
    }
  }
}

// TODO: update this to use DMA2D ASAP
void display_fill_rect(uint8_t color_index, uint16_t x, uint16_t y,
                       uint16_t width, uint16_t height) {
  uint16_t color565 = abgr8888_to_bgr565(palette[color_index]);

  if (width == 0 || height == 0) {
    return;
  }

  // Clip the rectangle to the screen dimensions
  if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) {
    return;
  }
  if (x + width > DISPLAY_WIDTH) {
    width = DISPLAY_WIDTH - x;
  }
  if (y + height > DISPLAY_HEIGHT) {
    height = DISPLAY_HEIGHT - y;
  }

  for (uint16_t j = y; j < y + height; j++) {
    for (uint16_t i = x; i < x + width; i++) {
      framebuffer[j * DISPLAY_WIDTH + i] = color565;
    }
  }
}
