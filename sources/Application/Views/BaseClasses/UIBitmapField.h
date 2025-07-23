/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _UI_BITMAP_FIELD_H_
#define _UI_BITMAP_FIELD_H_

#include "UIField.h"

class UIBitmapField : public UIField {
public:
  UIBitmapField(GUIPoint &position, uint16_t width, uint16_t height,
                const uint8_t *bitmap_data, uint16_t fg_color,
                uint16_t bg_color);

  virtual ~UIBitmapField();

  // Implement UIField virtual methods
  virtual void Draw(GUIWindow &w, int offset = 0);
  virtual void OnClick(); // ENTER pressed
  virtual void ProcessArrow(unsigned short mask);

  // Methods to update bitmap properties
  void SetBitmap(const uint8_t *bitmap_data);
  void SetColors(uint16_t fg_color, uint16_t bg_color);

private:
  uint16_t width_;
  uint16_t height_;
  const uint8_t *bitmap_data_;
  uint16_t fg_color_;
  uint16_t bg_color_;
};

#endif // _UI_BITMAP_FIELD_H_
