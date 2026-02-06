/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "DrawTools.h"
#include "Application/AppWindow.h"
#include "Foundation/Constants/SpecialCharacters.h"

void DrawBorder(View &view, int x, int y, int width, int height,
                const GUITextProperties &props, const char *charset) {
  char line[SCREEN_WIDTH + 1];
  memset(line, charset[4], width);
  line[width] = 0;

  line[0] = charset[0];         // top left
  line[width - 1] = charset[1]; // top right
  view.DrawString(x, y, line, props);

  line[0] = charset[2];         // bottom left
  line[width - 1] = charset[3]; // bottom right
  view.DrawString(x, y + height - 1, line, props);

  char vline[2] = {charset[5], 0}; // vertical line
  for (int i = 1; i < height - 1; i++) {
    view.DrawString(x, y + i, vline, props);
    view.DrawString(x + width - 1, y + i, vline, props);
  }
}

void DrawDoubleBorder(View &view, int x, int y, int width, int height,
                      const GUITextProperties &props) {
  DrawBorder(view, x, y, width, height, props, char_border_double_charset);
}

void DrawSingleBorder(View &view, int x, int y, int width, int height,
                      const GUITextProperties &props) {
  DrawBorder(view, x, y, width, height, props, char_border_single_charset);
}