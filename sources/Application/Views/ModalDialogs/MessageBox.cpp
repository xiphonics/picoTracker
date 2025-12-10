/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "MessageBox.h"
#include <Application/AppWindow.h>

static const char *buttonText[MBL_LAST] = {"Ok", "Yes", "Cancel", "No"};

MessageBox::MessageBox(View &view, const char *message, int btnFlags)
    : ModalView(view), line1_(message) {

  buttonCount_ = 0;
  for (int i = 0; i < MBL_LAST; i++) {
    if (btnFlags & (1 << (i))) {
      button_[buttonCount_] = i;
      buttonCount_++;
    }
  }
  selected_ = buttonCount_ - 1;
  NAssert(buttonCount_ != 0);
};

// Constructor for 2 line message
MessageBox::MessageBox(View &view, const char *messageLine1,
                       const char *messageLine2, int btnFlags)
    : ModalView(view), line1_(messageLine1), line2_(messageLine2) {

  buttonCount_ = 0;
  for (int i = 0; i < MBL_LAST; i++) {
    if (btnFlags & (1 << (i))) {
      button_[buttonCount_] = i;
      buttonCount_++;
    }
  }
  selected_ = buttonCount_ - 1;
  NAssert(buttonCount_ != 0);
}

MessageBox::~MessageBox(){};

void MessageBox::DrawView() {
  // message size
  int size1 = line1_.size();
  int size2 = line2_.size();
  int size = (size1 > size2) ? size1 : size2;

  // compute space needed for buttons
  // and set window size

  int btnSize = 5;
  int width = buttonCount_ * (btnSize + 1) + 1;
  width = (size > width) ? size : width;
  SetWindow(width, line2_.size() > 0 ? 4 : 3);

  // draw text
  int y = 0;
  int x = (width - size) / 2;
  GUITextProperties props;
  SetColor(CD_NORMAL);
  DrawString(x, y, line1_.c_str(), props);
  if (line2_.size() > 0) {
    y++;
    DrawString(x, y, line2_.c_str(), props);
  }

  y += 2;
  int offset = width / (buttonCount_ + 1);

  for (int i = 0; i < buttonCount_; i++) {
    const char *text = buttonText[button_[i]];
    x = offset * (i + 1) - strlen(text) / 2;
    if (i == selected_) {
      SetColor(CD_HILITE2);
      props.invert_ = true;
    } else {
      SetColor(CD_HILITE1);
      props.invert_ = false;
    }
    DrawString(x, y, text, props);
  }
};

void MessageBox::OnPlayerUpdate(PlayerEventType, unsigned int currentTick){};
void MessageBox::OnFocus(){};
void MessageBox::ProcessButtonMask(unsigned short mask, bool pressed) {
  if (mask & EPBM_LEFT) {
    selected_ = (selected_ + 1);
    if (selected_ >= buttonCount_) {
      selected_ = 0;
    }
  } else if (mask & EPBM_RIGHT) {
    selected_ = (selected_ - 1);
    if (selected_ < 0) {
      selected_ = buttonCount_ - 1;
    }
  } else if (mask & EPBM_ENTER && pressed) {
    EndModal(button_[selected_]);
  }
  isDirty_ = true;
};
