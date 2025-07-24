/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "picoBitmapGraphics.h"
#include "pico/stdlib.h"
#include <cstring>

void picoBitmapGraphics::drawBitmap(uint16_t x, uint16_t y, uint16_t width,
                                uint16_t height, const uint8_t *bitmap_data,
                                uint16_t fg_color, uint16_t bg_color) {
  /// Convert character cell coordinates to pixel coordinates
  uint16_t x_pixel = x * CHAR_WIDTH;
  uint16_t y_pixel = y * CHAR_HEIGHT;

  // Because of the rotated display swap x & y when sending commands to display
  // Use ILI9341_TFTWIDTH (240) for the calculation
  uint16_t display_x = ILI9341_TFTWIDTH - y_pixel - height;
  uint16_t display_y = x_pixel;

  // Width must be a multiple of 8 pixels for bitmap byte alignment
  // This is a requirement for our bitmap format
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
    uint16_t buffer[height];
    uint16_t *buffer_ptr = buffer;

    for (int row = 0; row < height; row++) {
      // Extract the bit from the bitmap data
      int byte_idx = col / 8;
      int bit_pos = 7 - (col % 8); // MSB first
      uint8_t byte_data = bitmap_data[row * bytes_per_row + byte_idx];
      bool pixel_set = (byte_data & (1 << bit_pos)) != 0;

      // Store the pixel color
      *buffer_ptr++ = pixel_set ? fg_color : bg_color;
    }

    // Write this column to the display
    picoDisplay.ili9341_write_data_continuous(buffer, height * sizeof(uint16_t));
  }

  ili9341_stop_writing();
}

void picoBitmapGraphics::clearBuffer(uint8_t *buffer, uint16_t width,
                                 uint16_t height) {
  assert(buffer != NULL);

  // For our bitmap format, width must be rounded up to the nearest multiple of
  // 8 to calculate the correct buffer size in bytes
  uint16_t bytes_per_row = (width + 7) / 8; // Round up to nearest byte
  uint16_t buffer_size = bytes_per_row * height;

  // Clear the buffer
  memset(buffer, 0, buffer_size);
}

void picoBitmapGraphics::setPixel(uint8_t *buffer, uint16_t width, uint16_t x,
                              uint16_t y, bool value) {
  assert(width % 8 == 0);
  assert(buffer != NULL);
  assert(x < width);

  // Calculate byte index and bit position
  uint16_t bytes_per_row = width / 8;
  uint16_t byte_idx = (y * bytes_per_row) + (x / 8);
  uint8_t bit_pos = 7 - (x % 8); // MSB first

  if (value) {
    // Set the bit
    buffer[byte_idx] |= (1 << bit_pos);
  } else {
    // Clear the bit
    buffer[byte_idx] &= ~(1 << bit_pos);
  }
}

bool picoBitmapGraphics::getPixel(const uint8_t *buffer, uint16_t width, uint16_t x,
                              uint16_t y) {
  assert(width % 8 == 0);
  assert(buffer != NULL);
  assert(x < width);

  // Calculate byte index and bit position
  uint16_t bytes_per_row = width / 8;
  uint16_t byte_idx = (y * bytes_per_row) + (x / 8);
  uint8_t bit_pos = 7 - (x % 8); // MSB first

  // Return the bit value
  return (buffer[byte_idx] & (1 << bit_pos)) != 0;
}

void picoBitmapGraphics::drawLine(uint8_t *buffer, uint16_t width, uint16_t height,
                              uint16_t x0, uint16_t y0, uint16_t x1,
                              uint16_t y1, bool value) {
  assert(width % 8 == 0);
  assert(buffer != NULL);

  // Bresenham's line algorithm
  int dx = abs(x1 - x0);
  int sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0);
  int sy = y0 < y1 ? 1 : -1;
  int err = dx + dy;
  int e2;

  while (1) {
    // Set pixel if it's within bounds
    if (x0 < width && y0 < height) {
      setPixel(buffer, width, x0, y0, value);
    }

    // Check if we've reached the end point
    if (x0 == x1 && y0 == y1) {
      break;
    }

    e2 = 2 * err;
    if (e2 >= dy) {
      if (x0 == x1)
        break;
      err += dy;
      x0 += sx;
    }
    if (e2 <= dx) {
      if (y0 == y1)
        break;
      err += dx;
      y0 += sy;
    }
  }
}

void picoBitmapGraphics::drawRect(uint8_t *buffer, uint16_t width, uint16_t height,
                              uint16_t x, uint16_t y, uint16_t rect_width,
                              uint16_t rect_height, bool filled, bool value) {
  assert(width % 8 == 0);
  assert(buffer != NULL);

  // Ensure we don't draw outside the bitmap
  if (x >= width)
    return;
  if (y >= height)
    return;

  // Clip rectangle if it extends beyond bitmap boundaries
  if (x + rect_width > width) {
    rect_width = width - x;
  }
  if (y + rect_height > height) {
    rect_height = height - y;
  }

  // If filled, draw each row of the rectangle
  if (filled) {
    for (uint16_t row = 0; row < rect_height; row++) {
      if (y + row < height) { // Check height bounds
        for (uint16_t col = 0; col < rect_width; col++) {
          if (x + col < width) { // Check width bounds
            setPixel(buffer, width, x + col, y + row, value);
          }
        }
      }
    }
  } else {
    // Draw the outline only
    // Top and bottom edges
    for (uint16_t col = 0; col < rect_width; col++) {
      if (x + col < width) {
        // Top edge
        setPixel(buffer, width, x + col, y, value);

        // Bottom edge (if in bounds)
        if (y + rect_height - 1 < height) {
    }

    // Left and right edges
    for (uint16_t row = 1; row < rect_height - 1; row++) {
      if (y + row < height) {
        // Left edge
        setPixel(buffer, width, x, y + row, value);

        // Right edge (if in bounds)
        if (x + rect_width - 1 < width) {
          setPixel(buffer, width, x + rect_width - 1, y + row,
                              value);
        }
      }
    }
  }
}
