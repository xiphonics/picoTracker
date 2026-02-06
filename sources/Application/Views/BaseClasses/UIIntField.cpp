/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "UIIntField.h"
#include "Application/AppWindow.h"
#include "System/Console/Trace.h"
#include "UIFramework/Interfaces/I_GUIGraphics.h"
#include "ViewUtils.h"
#include <System/Console/nanoprintf.h>

#define abs(x) (x < 0 ? -x : x)

UIIntField::UIIntField(const GUIPoint &position, int *src, const char *format,
                       int min, int max, int xOffset, int yOffset)
    : UIField(position) {
  src_ = src;
  format_ = format;
  min_ = min;
  max_ = max;
  xOffset_ = xOffset;
  yOffset_ = yOffset;
};

void UIIntField::Draw(GUIWindow &w) {

  GUITextProperties props;
  GUIPoint position = GetPosition();

  // ensure max field length
  char buffer[MAX_FIELD_WIDTH + 1];
  int value = *src_;
  npf_snprintf(buffer, sizeof(buffer), format_, value);

  if (focus_) {
    ((AppWindow &)w).SetColor(CD_HILITE2);
    props.invert_ = true;
    w.DrawString(buffer, position, props);
  } else {
    DrawLabeledField(w, position, buffer);
  }
};

void UIIntField::ProcessArrow(unsigned short mask) {

  int value = *src_;

  switch (mask) {
  case EPBM_UP:
    value += yOffset_;
    break;
  case EPBM_DOWN:
    value -= yOffset_;
    break;
  case EPBM_LEFT:
    value -= xOffset_;
    break;
  case EPBM_RIGHT:
    value += xOffset_;
    break;
  };
  if (value < min_) {
    value = min_;
  };
  if (value > max_) {
    value = max_;
  }

  *src_ = value;
};
