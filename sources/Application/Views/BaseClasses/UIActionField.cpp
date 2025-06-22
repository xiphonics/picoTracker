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

UIActionField::UIActionField(const char *name, unsigned int fourcc,
                             GUIPoint &position)
    : UIField(position) {
  name_ = name;
  fourcc_ = fourcc;
};

UIActionField::~UIActionField(){

};
void UIActionField::Draw(GUIWindow &w, int offset) {

  GUITextProperties props;
  GUIPoint position(x_, y_ + offset);

  if (focus_) {
    ((AppWindow &)w).SetColor(CD_HILITE2);
    props.invert_ = true;
  } else {
    ((AppWindow &)w).SetColor(CD_NORMAL);
  }

  w.DrawString(name_, position, props);
};

void UIActionField::OnClick() {
  SetChanged();
  NotifyObservers((I_ObservableData *)fourcc_);
};

const char *UIActionField::GetString() { return name_; };
