/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "FullScreenBox.h"
#include <Application/AppWindow.h>

static const char *buttonText[MBL_LAST] = {"Ok", "Yes", "Cancel", "No"};

FullScreenBox::FullScreenBox(View &view, const char *message, int btnFlags)
    : MessageBox(view, message, btnFlags) {}

FullScreenBox::FullScreenBox(View &view, const char *messageLine1,
                             const char *messageLine2, int btnFlags)
    : MessageBox(view, messageLine1, messageLine2, btnFlags) {}

FullScreenBox::~FullScreenBox(){};

void FullScreenBox::DrawView() {
  // message size
  int line1_width = line1_.size();
  // set window size full screen
  SetWindow(SCREEN_WIDTH, SCREEN_HEIGHT);

  // draw text
  int x1 = ((SCREEN_WIDTH - line1_width) / 2) - 2;
  int y1 = (SCREEN_HEIGHT / 2) - 4;
  GUITextProperties props;
  SetColor(CD_ERROR);
  DrawString(x1, y1, line1_.c_str(), props);
  if (line2_.size() > 0) {
    int line2_width = line2_.size();
    int x2 = ((SCREEN_WIDTH - line2_width) / 2) - 2;
    int y2 = y1 + 2;
    DrawString(x2, y2, line2_.c_str(), props);
  }
  SetColor(CD_NORMAL);
};
