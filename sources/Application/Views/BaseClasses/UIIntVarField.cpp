/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "UIIntVarField.h"

#include "Application/AppWindow.h"
#include "System/Console/Trace.h"
#include "UIFramework/Interfaces/I_GUIGraphics.h"
#include "UIIntVarField.h"
#include "ViewUtils.h"
#include <System/Console/nanoprintf.h>
#include <string.h>

#define abs(x) (x < 0 ? -x : x)

UIIntVarField::UIIntVarField(GUIPoint &position, Variable &v,
                             const char *format, int min, int max, int xOffset,
                             int yOffset, int displayOffset)
    : UIField(position), src_(v) {
  format_ = format;
  min_ = min;
  max_ = max;
  xOffset_ = xOffset;
  yOffset_ = yOffset;
  displayOffset_ = displayOffset;
};

void UIIntVarField::Draw(GUIWindow &w, int offset) {

  GUITextProperties props;
  GUIPoint position = GetPosition();
  position._y += offset;

  Variable::Type type = src_.GetType();
  char buffer[MAX_FIELD_WIDTH + 1];
  switch (type) {
  case Variable::INT: {
    int ivalue = src_.GetInt() + displayOffset_;
    npf_snprintf(buffer, sizeof(buffer), format_, ivalue, ivalue);
  } break;
  case Variable::CHAR_LIST:
    // if no value initialize with "NONE"
    if (src_.GetInt() < 0) {
      npf_snprintf(buffer, sizeof(buffer), format_, "NONE");
    } else {
      const char *cvalue = src_.GetString().c_str();
      npf_snprintf(buffer, sizeof(buffer), format_, cvalue);
    }
    break;
  case Variable::BOOL: {
    const char *cvalue = src_.GetString().c_str();
    npf_snprintf(buffer, sizeof(buffer), format_, cvalue);
  } break;

  default:
    strcpy(buffer, "++wtf++");
  }

  if (focus_) {
    ((AppWindow &)w).SetColor(CD_HILITE2);
    props.invert_ = true;
    w.DrawString(buffer, position, props);
  } else {
    DrawColoredField(w, position, buffer);
  }
};

void UIIntVarField::ProcessArrow(unsigned short mask) {
  int value = src_.GetInt();

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

  src_.SetInt(value);

  SetChanged();
  NotifyObservers((I_ObservableData *)(char)src_.GetID());
};

FourCC UIIntVarField::GetVariableID() { return src_.GetID(); };

Variable &UIIntVarField::GetVariable() { return src_; };
