/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "MessageBox.h"
#include "Foundation/Constants/SpecialCharacters.h"
#include "System/Console/n_assert.h"
#include <Application/AppWindow.h>
#include <new>

static const char *buttonText[MBL_LAST] = {"Ok", "Yes", "Cancel", "No"};

bool MessageBox::inUse_ = false;
alignas(MessageBox) static unsigned char MessageBoxStorage[sizeof(MessageBox)];
void *MessageBox::storage_ = MessageBoxStorage;

MessageBox *MessageBox::Create(View &view, const char *message, int btnFlags) {
  if (inUse_) {
    auto *existing = reinterpret_cast<MessageBox *>(storage_);
    existing->~MessageBox();
    inUse_ = false;
  }
  inUse_ = true;
  return new (storage_) MessageBox(view, message, btnFlags);
}

MessageBox *MessageBox::Create(View &view, const char *message,
                               const char *message2, int btnFlags) {
  if (inUse_) {
    auto *existing = reinterpret_cast<MessageBox *>(storage_);
    existing->~MessageBox();
    inUse_ = false;
  }
  inUse_ = true;
  return new (storage_) MessageBox(view, message, message2, btnFlags);
}

MessageBox::MessageBox(View &view, const char *message, int btnFlags)
    : ModalView(view), line1_(message) {
  InitButtons(btnFlags);
};

// Constructor for 2 line message
MessageBox::MessageBox(View &view, const char *messageLine1,
                       const char *messageLine2, int btnFlags)
    : ModalView(view), line1_(messageLine1), line2_(messageLine2) {
  InitButtons(btnFlags);
}

MessageBox::~MessageBox(){};

void MessageBox::InitButtons(int btnFlags) {
  buttonCount_ = 0;
  buttonWidth_ = 0;

  for (int i = 0; i < MBL_LAST; i++) {
    if (btnFlags & (1 << (i))) {
      button_[buttonCount_] = i;
      buttonCount_++;

      int len = strlen(buttonText[button_[i]]);
      buttonWidth_ = std::max(buttonWidth_, len);
    }
  }
  selected_ = buttonCount_ - 1;
  NAssert(buttonCount_ != 0);
}

void MessageBox::Destroy() {
  this->~MessageBox();
  inUse_ = false;
}

void MessageBox::DrawView() {
  // message size
  int size = std::max(line1_.size(), line2_.size());

  // compute space needed for buttons
  // and set window size
  int width = buttonCount_ * (buttonWidth_ + 1) + 1;
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
    bool selected = (i == selected_);

    const char *text = buttonText[button_[i]];
    x = offset * (i + 1) - strlen(text) / 2;
    SetColor(selected ? CD_HILITE2 : CD_HILITE1);
    props.invert_ = selected;

    DrawString(x, y, text, props);

    // button ends
    if (selected) {
      props.invert_ = false;
      DrawString(x - 1, y, char_button_border_left_s, props);
      DrawString(x + strlen(text), y, char_button_border_right_s, props);
    }
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
