/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "advBitmapGraphics.h"
#include "display.h"
#include "dma2d.h"
#include <cassert>
#include <cmath>   // For std::abs
#include <cstring> // For std::memset

/**
 * @file advBitmapGraphics.cpp
 * @brief Implements the BitmapGraphics interface for an STM32 device
 * with direct framebuffer access.
 */

#define CHAR_WIDTH 22
#define CHAR_HEIGHT 30

void advBitmapGraphics::setPixel(uint8_t *buffer, uint16_t width, uint16_t x,
                                 uint16_t y, bool value) {
  // Ensure we are drawing within the horizontal bounds of the buffer
  if (x >= width) {
    return;
  }

  // Calculate which byte contains the pixel
  uint16_t bytes_per_row = width / 8;
  uint32_t byte_index = (y * bytes_per_row) + (x / 8);

  // Calculate the specific bit within that byte (MSB-first format)
  uint8_t bit_position = 7 - (x % 8);

  if (value) {
    // Set the bit to 1
    buffer[byte_index] |= (1 << bit_position);
  } else {
    // Clear the bit to 0
    buffer[byte_index] &= ~(1 << bit_position);
  }
}

bool advBitmapGraphics::getPixel(const uint8_t *buffer, uint16_t width,
                                 uint16_t x, uint16_t y) {
  if (x >= width) {
    return false;
  }

  uint16_t bytes_per_row = width / 8;
  uint32_t byte_index = (y * bytes_per_row) + (x / 8);
  uint8_t bit_position = 7 - (x % 8);

  // Return true if the bit is set, false otherwise
  return (buffer[byte_index] & (1 << bit_position)) != 0;
}

void advBitmapGraphics::clearBuffer(uint8_t *buffer, uint16_t width,
                                    uint16_t height) {
  uint32_t buffer_size_bytes = (width / 8) * height;
  std::memset(buffer, 0, buffer_size_bytes);
}

void advBitmapGraphics::drawLine(uint8_t *buffer, uint16_t width,
                                 uint16_t height, uint16_t x0, uint16_t y0,
                                 uint16_t x1, uint16_t y1, bool value) {
  int dx = std::abs(x1 - x0);
  int sx = x0 < x1 ? 1 : -1;
  int dy = -std::abs(y1 - y0);
  int sy = y0 < y1 ? 1 : -1;
  int err = dx + dy;

  while (true) {
    // Set the current pixel if it's within the buffer bounds
    if (x0 < width && y0 < height) {
      setPixel(buffer, width, x0, y0, value);
    }
    if (x0 == x1 && y0 == y1) {
      break;
    }
    int e2 = 2 * err;
    if (e2 >= dy) {
      err += dy;
      x0 += sx;
    }
    if (e2 <= dx) {
      err += dx;
      y0 += sy;
    }
  }
}

void advBitmapGraphics::drawRect(uint8_t *buffer, uint16_t width,
                                 uint16_t height, uint16_t x, uint16_t y,
                                 uint16_t rect_width, uint16_t rect_height,
                                 bool filled, bool value) {
  if (filled) {
    // For a filled rectangle, draw every pixel inside its bounds
    for (uint16_t i = 0; i < rect_height; ++i) {
      for (uint16_t j = 0; j < rect_width; ++j) {
        setPixel(buffer, width, x + j, y + i, value);
      }
    }
  } else {
    // For an outline, draw four separate lines
    if (rect_width > 0 && rect_height > 0) {
      uint16_t x_end = x + rect_width - 1;
      uint16_t y_end = y + rect_height - 1;
      drawLine(buffer, width, height, x, y, x_end, y, value);         // Top
      drawLine(buffer, width, height, x, y_end, x_end, y_end, value); // Bottom
      drawLine(buffer, width, height, x, y, x, y_end, value);         // Left
      drawLine(buffer, width, height, x_end, y, x_end, y_end, value); // Right
    }
  }
}

void advBitmapGraphics::drawBitmap(uint16_t x, uint16_t y, uint16_t width,
                                   uint16_t height, const uint8_t *bitmap_data,
                                   uint16_t fg_color, uint16_t bg_color) {
  // Convert character cell coordinates to the top-left pixel coordinate on
  // screen
  uint16_t start_x = x * CHAR_WIDTH;
  uint16_t start_y = y * CHAR_HEIGHT;

  // Pre-convert the 16-bit colors to the 32-bit format of the framebuffer
  uint32_t final_fg_color = rgb565_to_abgr8888(fg_color);
  uint32_t final_bg_color = rgb565_to_abgr8888(bg_color);

  // Iterate over every pixel of the source bitmap
  for (uint16_t h = 0; h < height; ++h) {
    uint16_t dest_y = start_y + h;

    // Clip vertically: if we're outside the screen, stop drawing
    if (dest_y >= FRAMEBUFFER_HEIGHT) {
      break;
    }

    for (uint16_t w = 0; w < width; ++w) {
      uint16_t dest_x = start_x + w;

      // Clip horizontally: if we're outside the screen, skip this pixel
      if (dest_x >= FRAMEBUFFER_WIDTH) {
        continue;
      }

      // Read the bit from the source bitmap to see if the pixel is "on" or
      // "off"
      bool pixel_is_set = getPixel(bitmap_data, width, w, h);

      // Select the appropriate color and write it directly to the framebuffer
      if (pixel_is_set) {
        framebuffer[dest_y * FRAMEBUFFER_WIDTH + dest_x] = final_fg_color;
      } else {
        framebuffer[dest_y * FRAMEBUFFER_WIDTH + dest_x] = final_bg_color;
      }
    }
  }
}