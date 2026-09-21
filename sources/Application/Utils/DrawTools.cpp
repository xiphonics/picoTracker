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

#include <string.h>

void DrawBorder(View &view, int x, int y, int width, int height,
                const GUITextProperties &props, const char *charset) {
  size_t len = strlen(charset);
  char hTop = charset[4];
  char hBottom = (len >= 8) ? charset[5] : charset[4];
  char vLeft = (len >= 8) ? charset[6] : charset[5];
  char vRight = (len >= 8) ? charset[7] : charset[5];

  char line[SCREEN_WIDTH + 1];
  memset(line, hTop, width);
  line[width] = 0;

  line[0] = charset[0];         // top left
  line[width - 1] = charset[1]; // top right
  view.DrawString(x, y, line, props);

  memset(line, hBottom, width);
  line[width] = 0;

  line[0] = charset[2];         // bottom left
  line[width - 1] = charset[3]; // bottom right
  view.DrawString(x, y + height - 1, line, props);

  char vlineLeft[2] = {vLeft, 0};
  char vlineRight[2] = {vRight, 0};
  for (int i = 1; i < height - 1; i++) {
    view.DrawString(x, y + i, vlineLeft, props);
    view.DrawString(x + width - 1, y + i, vlineRight, props);
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

void DrawSolidBorder(View &view, int x, int y, int width, int height,
                     const GUITextProperties &props) {
  DrawBorder(view, x, y, width, height, props, char_border_solid_charset);
}
