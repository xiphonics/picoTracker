/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _PICO_BITMAPGRAPHICS_H
#define _PICO_BITMAPGRAPHICS_H

#include "System/Display/BitmapGraphics.h"
#include "chargfx.h"
#include <cstdint>

class picoBitmapGraphics : public BitmapGraphics {
public:
  virtual void drawBitmap(uint16_t x, uint16_t y, uint16_t width,
                          uint16_t height, const uint8_t *bitmap_data,
                          uint16_t fg_color, uint16_t bg_color) override;

  virtual void clearBuffer(uint8_t *buffer, uint16_t width,
                           uint16_t height) override;

  virtual void setPixel(uint8_t *buffer, uint16_t width, uint16_t x, uint16_t y,
                        bool value) override;

  virtual bool getPixel(const uint8_t *buffer, uint16_t width, uint16_t x,
                        uint16_t y) override;

  virtual void drawLine(uint8_t *buffer, uint16_t width, uint16_t height,
                        uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                        bool value) override;

  virtual void drawRect(uint8_t *buffer, uint16_t width, uint16_t height,
                        uint16_t x, uint16_t y, uint16_t rect_width,
                        uint16_t rect_height, bool filled, bool value) override;
};

#endif
