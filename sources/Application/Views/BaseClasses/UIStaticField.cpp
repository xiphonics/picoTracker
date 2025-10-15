/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "UIStaticField.h"
#include "Application/AppWindow.h"
#include "ViewUtils.h"
#include <string.h>

UIStaticField::UIStaticField(GUIPoint &position, const char *string)
    : UIField(position) {
  string_ = string;
};

void UIStaticField::Draw(GUIWindow &w, int offset) {

  GUIPoint position = GetPosition();
  position._y += offset;

  char buffer[MAX_FIELD_WIDTH + 1];
  strncpy(buffer, string_, MAX_FIELD_WIDTH);
  buffer[MAX_FIELD_WIDTH] = '\0';
  DrawColoredField(w, position, buffer);
};

void UIStaticField::ProcessArrow(unsigned short mask){};

bool UIStaticField::IsStatic() { return true; };
