/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "UIBitmapField.h"
#include "Adapters/picoTracker/display/bitmapgfx.h"

UIBitmapField::UIBitmapField(GUIPoint &position, uint16_t width,
                             uint16_t height, const uint8_t *bitmap_data,
                             uint16_t fg_color, uint16_t bg_color)
    : UIField(position), width_(width), height_(height),
      bitmap_data_(bitmap_data), fg_color_(fg_color), bg_color_(bg_color) {}

UIBitmapField::~UIBitmapField() {
  // No dynamic memory to clean up
}

void UIBitmapField::Draw(GUIWindow &w, int offset) {
  // Draw the bitmap using our new chargfx_draw_bitmap function
  bitmapgfx_draw_bitmap(x_, y_, width_, height_, bitmap_data_, fg_color_,
                        bg_color_);
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
