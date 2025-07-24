/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "advBitmapGraphics.h"

void advBitmapGraphics::drawBitmap(uint16_t x, uint16_t y, uint16_t width,
                                   uint16_t height, const uint8_t *bitmap_data,
                                   uint16_t fg_color, uint16_t bg_color) {
  // TODO
}

void advBitmapGraphics::clearBuffer(uint8_t *buffer, uint16_t width,
                                    uint16_t height) {
  // TODO
}

void advBitmapGraphics::setPixel(uint8_t *buffer, uint16_t width, uint16_t x,
                                 uint16_t y, bool value) {
  // TODO
}

bool advBitmapGraphics::getPixel(const uint8_t *buffer, uint16_t width,
                                 uint16_t x, uint16_t y) {
  // TODO
  return false;
}

void advBitmapGraphics::drawLine(uint8_t *buffer, uint16_t width,
                                 uint16_t height, uint16_t x0, uint16_t y0,
                                 uint16_t x1, uint16_t y1, bool value) {
  // TODO
}

void advBitmapGraphics::drawRect(uint8_t *buffer, uint16_t width,
                                 uint16_t height, uint16_t x, uint16_t y,
                                 uint16_t rect_width, uint16_t rect_height,
                                 bool filled, bool value) {
  // TODO
}
