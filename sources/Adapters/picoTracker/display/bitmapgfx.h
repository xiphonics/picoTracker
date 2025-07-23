/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _BITMAPGFX_H
#define _BITMAPGFX_H

#include "chargfx.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Draw a monochrome bitmap on the display
 * @param x X position in character cells
 * @param y Y position in character cells
 * @param width Width in pixels (must be multiple of 8)
 * @param height Height in pixels
 * @param bitmap_data Pointer to bitmap data (1 bit per pixel, row-major order)
 * @param fg_color Foreground color (for 1 bits)
 * @param bg_color Background color (for 0 bits)
 */
void bitmapgfx_draw_bitmap(uint16_t x, uint16_t y, uint16_t width,
                           uint16_t height, const uint8_t *bitmap_data,
                           uint16_t fg_color, uint16_t bg_color);

/**
 * Clear a bitmap buffer (set all pixels to 0)
 * @param buffer Pointer to the bitmap buffer
 * @param width Width in pixels (must be multiple of 8)
 * @param height Height in pixels
 */
void bitmapgfx_clear_buffer(uint8_t *buffer, uint16_t width, uint16_t height);

/**
 * Set a pixel in a bitmap buffer
 * @param buffer Pointer to the bitmap buffer
 * @param width Width of the bitmap in pixels (must be multiple of 8)
 * @param x X coordinate of the pixel
 * @param y Y coordinate of the pixel
 * @param value Pixel value (true = set, false = clear)
 */
void bitmapgfx_set_pixel(uint8_t *buffer, uint16_t width, uint16_t x,
                         uint16_t y, bool value);

/**
 * Get a pixel from a bitmap buffer
 * @param buffer Pointer to the bitmap buffer
 * @param width Width of the bitmap in pixels (must be multiple of 8)
 * @param x X coordinate of the pixel
 * @param y Y coordinate of the pixel
 * @return Pixel value (true = set, false = clear)
 */
bool bitmapgfx_get_pixel(const uint8_t *buffer, uint16_t width, uint16_t x,
                         uint16_t y);

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
void bitmapgfx_draw_line(uint8_t *buffer, uint16_t width, uint16_t height,
                         uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                         bool value);

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
void bitmapgfx_draw_rect(uint8_t *buffer, uint16_t width, uint16_t height,
                         uint16_t x, uint16_t y, uint16_t rect_width,
                         uint16_t rect_height, bool filled, bool value);

#ifdef __cplusplus
}
#endif

#endif // _BITMAPGFX_H