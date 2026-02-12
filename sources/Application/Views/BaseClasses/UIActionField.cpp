/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "UIActionField.h"
#include "Application/AppWindow.h"
#include "Foundation/Constants/SpecialCharacters.h"
#include "ViewUtils.h"
#include <string.h>

UIActionField::UIActionField(const char *name, unsigned int fourcc,
                             GUIPoint &position)
    : UIField(position) {
  name_ = name;
  fourcc_ = fourcc;
};

UIActionField::~UIActionField(){};

void UIActionField::Draw(GUIWindow &w, int offset) {

  GUITextProperties props;

  GUIPoint position(x_, y_ + offset);

  if (focus_) {
    ((AppWindow &)w).SetColor(CD_HILITE2);
    props.invert_ = true;
    w.DrawString(name_, position, props);
    props.invert_ = false;
    w.DrawString(char_button_border_left_s, position - GUIPoint(1, 0), props);
    w.DrawString(char_button_border_right_s,
                 position + GUIPoint(strlen(name_), 0), props);

  } else {
    // enforce max field length
    char buffer[MAX_FIELD_WIDTH + 1];
    strncpy(buffer, name_, MAX_FIELD_WIDTH);
    buffer[MAX_FIELD_WIDTH] = '\0';

    ((AppWindow &)w).SetColor(CD_HILITE1);
    w.DrawString(buffer, position, props);
  }
};

void UIActionField::OnClick() {
  SetChanged();
  NotifyObservers((I_ObservableData *)fourcc_);
};

const char *UIActionField::GetString() { return name_; };
