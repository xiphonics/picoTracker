/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "UIBitmapField.h"
#include "System/Display/BitmapGraphics.h"

UIBitmapField::UIBitmapField(GUIPoint &position, uint16_t width,
                             uint16_t height, const uint8_t *bitmap_data,
                             uint16_t fg_color, uint16_t bg_color)
    : UIField(position), width_(width), height_(height),
      bitmap_data_(bitmap_data), fg_color_(fg_color), bg_color_(bg_color) {}

UIBitmapField::~UIBitmapField() {
  // No dynamic memory to clean up
}

void UIBitmapField::Draw(GUIWindow &w, int offset) {
  BitmapGraphics *gfx = BitmapGraphics::GetInstance();
  if (gfx == nullptr) {
    Trace::Error("BitmapGraphics instance is null");
    return;
  }

#ifdef ADV
  const int scale = 2;
  uint16_t scaled_width = width_ * scale;
  uint16_t scaled_height = height_ * scale;

  // Ensure scaled_width is a multiple of 8 for byte alignment
  int bytes_per_row = (scaled_width + 7) / 8;
  uint16_t buffer_size = bytes_per_row * scaled_height;
  uint8_t *scaled_buffer = new uint8_t[buffer_size];
  gfx->clearBuffer(scaled_buffer, scaled_width, scaled_height);

  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      if (gfx->getPixel(bitmap_data_, width_, x, y)) {
        int sx = x * scale;
        int sy = y * scale;
        for (int i = 0; i < scale; ++i) {
          for (int j = 0; j < scale; ++j) {
            gfx->setPixel(scaled_buffer, scaled_width, sx + i, sy + j, true);
          }
        }
      }
    }
  }

  gfx->drawBitmap(x_, y_, scaled_width, scaled_height, scaled_buffer, fg_color_,
                  bg_color_);

  delete[] scaled_buffer;
#else
  gfx->drawBitmap(x_, y_, width_, height_, bitmap_data_, fg_color_, bg_color_);
#endif
}

void UIBitmapField::OnClick() {
  // Default implementation - can be overridden by derived classes
  // For now, just toggle focus
  if (HasFocus()) {
    ClearFocus();
  } else {
    SetFocus();
  }
}

void UIBitmapField::ProcessArrow(unsigned short mask) {
  // Default implementation - can be overridden by derived classes
  // No default behavior for arrow keys
}

void UIBitmapField::SetBitmap(const uint8_t *bitmap_data) {
  bitmap_data_ = bitmap_data;
}

void UIBitmapField::SetColors(uint16_t fg_color, uint16_t bg_color) {
  fg_color_ = fg_color;
  bg_color_ = bg_color;
}
