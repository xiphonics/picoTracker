/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "ConsoleView.h"
#include <string.h>

ConsoleView::ConsoleView(GUIWindow &w, ViewData *viewData) : View(w, viewData) {
  currentLine_ = 0;
  for (int i = 0; i < CONSOLE_HEIGHT; i++) {
    lines_[i][0] = 0;
  }
  isDirty_ = true;
};

void ConsoleView::ProcessButtonMask(unsigned short mask, bool pressed){};

void ConsoleView::DrawView() {

  GUITextProperties props;

  SetColor(CD_NORMAL);
  GUIPoint pos(0, 0);
  for (int i = 0; i < CONSOLE_HEIGHT; i++) {
    w_.DrawString(lines_[(currentLine_ + i) % CONSOLE_HEIGHT], pos, props);
    pos._y += 8;
  }
};

bool ConsoleView::IsDirty() { return isDirty_; };

void ConsoleView::AddBuffer(char *buffer){};
