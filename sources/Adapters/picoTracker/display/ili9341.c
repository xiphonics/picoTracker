/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "ili9341.h"
#include "Adapters/picoTracker/platform/gpio.h"
#include "pico/stdlib.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static inline void cs_select() { gpio_put(DISPLAY_CS, 0); }

static inline void cs_deselect() { gpio_put(DISPLAY_CS, 1); }

void ili9341_set_command(uint8_t cmd) {
  cs_select();
  gpio_put(DISPLAY_DC, 0);
  spi_write_blocking(DISPLAY_SPI, &cmd, 1);
  gpio_put(DISPLAY_DC, 1);
  cs_deselect();
}

void ili9341_command_param16(uint16_t data) {
  ili9341_command_param(data >> 8);
  ili9341_command_param(data & 0xFF);
}

void ili9341_command_param(uint8_t data) {
  cs_select();
  spi_write_blocking(DISPLAY_SPI, &data, 1);
  cs_deselect();
}

inline void ili9341_start_writing() { cs_select(); }

void ili9341_write_data(void *buffer, int bytes) {
  cs_select();
  spi_write_blocking(DISPLAY_SPI, buffer, bytes);
  cs_deselect();
}

void ili9341_write_data_continuous(void *buffer, int bytes) {
  spi_write_blocking(DISPLAY_SPI, buffer, bytes);
}

inline void ili9341_stop_writing() { cs_deselect(); }

void ili9341_init() {

  sleep_ms(10);
  gpio_put(DISPLAY_RESET, 0);
  sleep_ms(10);
  gpio_put(DISPLAY_RESET, 1);

  ili9341_set_command(0x01); // soft reset
  sleep_ms(100);

  ili9341_set_command(ILI9341_GAMMASET);
  ili9341_command_param(0x01);

#ifdef LCD_ST7789
  // gamma correction for ST7789V
  // src:
  // https://github.com/kiklhorn/esphome/blob/dbb824c937fda160c0f2165a29a8b1a9aee4fb43/esphome/components/st7789/st7789_init.h#L57
  // positive gamma correction
  ili9341_set_command(ILI9341_GMCTRP1);
  ili9341_write_data((uint8_t[14]){0xD0, 0x00, 0x02, 0x07, 0x0A, 0x28, 0x32,
                                   0x44, 0x42, 0x06, 0x0E, 0x12, 0x14, 0x17},
                     14);
  // negative gamma correction
  ili9341_set_command(ILI9341_GMCTRN1);
  ili9341_write_data((uint8_t[14]){0xD0, 0x00, 0x02, 0x07, 0x0A, 0x28, 0x31,
                                   0x54, 0x47, 0x0E, 0x1C, 0x17, 0x1B, 0x1E},
                     14);

#else
  // positive gamma correction
  ili9341_set_command(ILI9341_GMCTRP1);
  ili9341_write_data((uint8_t[16]){0x0f, 0x31, 0x2b, 0x0c, 0x0e, 0x08, 0x4e,
                                   0xf1, 0x37, 0x07, 0x10, 0x03, 0x0e, 0x09,
                                   0x00},
                     15);

  // negative gamma correction
  ili9341_set_command(ILI9341_GMCTRN1);
  ili9341_write_data((uint8_t[15]){0x00, 0x0e, 0x14, 0x03, 0x11, 0x07, 0x31,
                                   0xc1, 0x48, 0x08, 0x0f, 0x0c, 0x31, 0x36,
                                   0x0f},
                     15);
#endif

  // memory access control
  ili9341_set_command(ILI9341_MADCTL);
  ili9341_command_param(LCD_MADCTL_DEFAULT);

#ifdef LCD_ST7789
  // correct orientation and invert colors for ST7789
  ili9341_set_command(ILI9341_INVON);
#endif

  // pixel format
  ili9341_set_command(ILI9341_PIXFMT);
  ili9341_command_param(0x55); // 16-bit

  // frame rate; default, 70 Hz
  ili9341_set_command(ILI9341_FRMCTR1);
  ili9341_command_param(0x00);
  ili9341_command_param(FRAMERATE_75);

  // exit sleep
  ili9341_set_command(ILI9341_SLPOUT);

  // display on
  ili9341_set_command(ILI9341_DISPON);

  // column address set
  ili9341_set_command(ILI9341_CASET);
  ili9341_command_param(0x00);
  ili9341_command_param(0x00); // start column
  ili9341_command_param(0x00);
  ili9341_command_param(0xef); // end column -> 239

  // page address set
  ili9341_set_command(ILI9341_PASET);
  ili9341_command_param(0x00);
  ili9341_command_param(0x00); // start page
  ili9341_command_param(0x01);
  ili9341_command_param(0x3f); // end page -> 319

  ili9341_set_command(ILI9341_RAMWR);
}

uint16_t swap_bytes(uint16_t color) { return (color >> 8) | (color << 8); }
