/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "UIField.h"

UIField::UIField(const GUIPoint &position) {
  x_ = position._x;
  y_ = position._y;
  focus_ = false;
};

UIField::~UIField(){};

GUIPoint UIField::GetPosition() {
  GUIPoint point(x_, y_);
  return point;
}

void UIField::SetPosition(const GUIPoint &p) {
  x_ = p._x;
  y_ = p._y;
};

void UIField::ClearFocus() { focus_ = false; };

void UIField::SetFocus() { focus_ = true; };

bool UIField::HasFocus() { return focus_; };

bool UIField::IsStatic() { return false; };
