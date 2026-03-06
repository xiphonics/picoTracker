/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "WarnMessageBox.h"
#include <new>
#include <string.h>

static const char *warnButtonText[MBL_LAST] = {"Ok", "Yes", "Cancel", "No"};

bool WarnMessageBox::inUse_ = false;
alignas(WarnMessageBox) static unsigned char WarnMessageBoxStorage
    [sizeof(WarnMessageBox)];
void *WarnMessageBox::storage_ = WarnMessageBoxStorage;

WarnMessageBox *WarnMessageBox::Create(View &view, const char *message,
                                       int btnFlags) {
  if (inUse_) {
    auto *existing = reinterpret_cast<WarnMessageBox *>(storage_);
    existing->~WarnMessageBox();
    inUse_ = false;
  }
  inUse_ = true;
  return new (storage_) WarnMessageBox(view, message, btnFlags);
}

WarnMessageBox *WarnMessageBox::Create(View &view, const char *message,
                                       const char *message2, int btnFlags) {
  if (inUse_) {
    auto *existing = reinterpret_cast<WarnMessageBox *>(storage_);
    existing->~WarnMessageBox();
    inUse_ = false;
  }
  inUse_ = true;
  return new (storage_) WarnMessageBox(view, message, message2, btnFlags);
}

WarnMessageBox::WarnMessageBox(View &view, const char *message, int btnFlags)
    : MessageBox(view, message, btnFlags) {}

WarnMessageBox::WarnMessageBox(View &view, const char *message,
                               const char *message2, int btnFlags)
    : MessageBox(view, message, message2, btnFlags) {}

WarnMessageBox::~WarnMessageBox() {}

void WarnMessageBox::Destroy() {
  this->~WarnMessageBox();
  inUse_ = false;
}

void WarnMessageBox::DrawView() {
  int size1 = line1_.size();
  int size2 = line2_.size();
  int size = (size1 > size2) ? size1 : size2;

  int btnSize = 5;
  int width = buttonCount_ * (btnSize + 1) + 1;
  width = (size > width) ? size : width;
  int height = line2_.size() > 0 ? 4 : 3;
  SetWindow(width, height);

  GUITextProperties props;
  props.invert_ = true;

  SetColor(CD_WARN);
  char fillLine[SCREEN_WIDTH + 1];
  int fillWidth = width + 2;
  memset(fillLine, ' ', fillWidth);
  fillLine[fillWidth] = '\0';
  for (int fy = -1; fy <= height; fy++) {
    DrawString(-1, fy, fillLine, props);
  }

  SetColor(CD_ERROR);
  char borderHoriz[SCREEN_WIDTH + 1];
  int borderWidth = width + 4;
  memset(borderHoriz, ' ', borderWidth);
  borderHoriz[borderWidth] = '\0';
  DrawString(-2, -2, borderHoriz, props);
  DrawString(-2, height + 1, borderHoriz, props);
  char borderVert[2] = {' ', '\0'};
  for (int by = -1; by <= height; by++) {
    DrawString(-2, by, borderVert, props);
    DrawString(width + 1, by, borderVert, props);
  }

  int y = 0;
  int x = (width - size) / 2;
  SetColor(CD_ERROR);
  DrawString(x, y, line1_.c_str(), props);
  if (line2_.size() > 0) {
    y++;
    DrawString(x, y, line2_.c_str(), props);
  }

  y += 2;
  int offset = width / (buttonCount_ + 1);
  for (int i = 0; i < buttonCount_; i++) {
    const char *text = warnButtonText[button_[i]];
    x = offset * (i + 1) - strlen(text) / 2;

    if (i == selected_) {
      props.invert_ = true;
      if (button_[i] == MBL_YES) {
        SetColor(CD_ERROR);
      } else {
        SetColor(CD_HILITE2);
      }
    } else {
      props.invert_ = false;
      SetColor(CD_HILITE1);
    }
    DrawString(x, y, text, props);
  }
}
