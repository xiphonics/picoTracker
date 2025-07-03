/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "bitmapgfx.h"
#include "chargfx.h"
#include "ili9341.h"
#include <assert.h>
#include <stdint.h>

// Draw a bitmap at the specified position
// x and y are in character cells
// width and height are in pixels
// pixel_data is a pointer to the bitmap data, each byte is 8 monochrome pixels
// of a column, MSB first
// fg_color and bg_color are the colors to use for the bitmap
void bitmapgfx_draw_bitmap(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                           const uint8_t *pixel_data, uint16_t fg_color,
                           uint16_t bg_color) {
  // Convert character cell coordinates to pixel coordinates
  uint16_t x_pixel = x * CHAR_WIDTH;
  uint16_t y_pixel = y * CHAR_HEIGHT;

  // Because of the rotated display swap x & y when sending commands to display
  uint16_t display_x = 240 - y_pixel - height;
  uint16_t display_y = x_pixel;

  // Ensure width is a multiple of 8 pixels for bitmap byte alignment because
  assert(width % 8 == 0);
  int bytes_per_row = width / 8;

  // Set display window with swapped dimensions for rotation
  ili9341_set_command(ILI9341_CASET);
  ili9341_command_param16(display_x);
  ili9341_command_param16(display_x + height - 1);

  ili9341_set_command(ILI9341_PASET);
  ili9341_command_param16(display_y);
  ili9341_command_param16(display_y + width - 1);

  ili9341_set_command(ILI9341_RAMWR);
  ili9341_start_writing();

  // Process bitmap column by column for rotated display
  for (int col = 0; col < width; col++) {
    uint16_t *buffer_ptr = buffer;

    for (int row = 0; row < height; row++) {
      // Extract the bit from the bitmap data
      int byte_idx = col / 8;
      int bit_pos = 7 - (col % 8); // MSB first
      uint8_t byte_data = pixel_data[row * bytes_per_row + byte_idx];
      bool pixel_set = (byte_data & (1 << bit_pos)) != 0;

      // Store the pixel color
      *buffer_ptr++ = pixel_set ? fg_color : bg_color;
    }

    // Write this column to the display
    ili9341_write_data_continuous(buffer, height * sizeof(uint16_t));
  }

  ili9341_stop_writing();
}