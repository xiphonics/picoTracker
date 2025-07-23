/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _BITMAPGRAPHICS_H
#define _BITMAPGRAPHICS_H

#include "Foundation/T_Factory.h"
#include <cstdint>

class BitmapGraphics : public T_Factory<BitmapGraphics> {
public:
  BitmapGraphics() = default;
  virtual ~BitmapGraphics() = default;

  /**
   * Draw a monochrome bitmap on the display
   * @param x X position in character cells
   * @param y Y position in character cells
   * @param width Width in pixels (must be multiple of 8)
   * @param height Height in pixels
   * @param bitmap_data Pointer to bitmap data (1 bit per pixel, row-major
   * order)
   * @param fg_color Foreground color (for 1 bits)
   * @param bg_color Background color (for 0 bits)
   */
  virtual void drawBitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                  const uint8_t *bitmap_data, uint16_t fg_color,
                  uint16_t bg_color) = 0;

  /**
   * Clear a bitmap buffer (set all pixels to 0)
   * @param buffer Pointer to the bitmap buffer
   * @param width Width in pixels (must be multiple of 8)
   * @param height Height in pixels
   */
  virtual void clearBuffer(uint8_t *buffer, uint16_t width, uint16_t height) = 0;

  /**
   * Set a pixel in a bitmap buffer
   * @param buffer Pointer to the bitmap buffer
   * @param width Width of the bitmap in pixels (must be multiple of 8)
   * @param x X coordinate of the pixel
   * @param y Y coordinate of the pixel
   * @param value Pixel value (true = set, false = clear)
   */
  virtual void setPixel(uint8_t *buffer, uint16_t width, uint16_t x, uint16_t y,
                bool value) = 0;

  /**
   * Get a pixel from a bitmap buffer
   * @param buffer Pointer to the bitmap buffer
   * @param width Width of the bitmap in pixels (must be multiple of 8)
   * @param x X coordinate of the pixel
   * @param y Y coordinate of the pixel
   * @return Pixel value (true = set, false = clear)
   */
  virtual bool getPixel(const uint8_t *buffer, uint16_t width, uint16_t x, uint16_t y) = 0;

  /**
   * Draw a line in a bitmap buffer using Bresenham's algorithm
   * @param buffer Pointer to the bitmap buffer
   * @param width Width of the bitmap in pixels (must be multiple of 8)
   * @param height Height of the bitmap in pixels
   * @param x0 Starting X coordinate
   * @param y0 Starting Y coordinate
   * @param x1 Ending X coordinate
   * @param y1 Ending Y coordinate
   * @param value Pixel value (true = set, false = clear)
   */
  virtual void drawLine(uint8_t *buffer, uint16_t width, uint16_t height, uint16_t x0,
                uint16_t y0, uint16_t x1, uint16_t y1, bool value) = 0;

  /**
   * Draw a rectangle in a bitmap buffer
   * @param buffer Pointer to the bitmap buffer
   * @param width Width of the bitmap in pixels (must be multiple of 8)
   * @param height Height of the bitmap in pixels
   * @param x X coordinate of the top-left corner
   * @param y Y coordinate of the top-left corner
   * @param rect_width Width of the rectangle
   * @param rect_height Height of the rectangle
   * @param filled Whether to fill the rectangle
   * @param value Pixel value (true = set, false = clear)
   */
  virtual void drawRect(uint8_t *buffer, uint16_t width, uint16_t height, uint16_t x,
                uint16_t y, uint16_t rect_width, uint16_t rect_height,
                bool filled, bool value) = 0;
};

#endif // _BITMAPGRAPHICS_H
