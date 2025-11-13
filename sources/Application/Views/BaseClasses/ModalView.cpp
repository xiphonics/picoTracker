/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "ModalView.h"

#include "Foundation/Constants/SpecialCharacters.h"

ModalView::ModalView(View &v)
    : View(v.w_, v.viewData_), finished_(false), returnCode_(0){};

ModalView::~ModalView(){};

int ModalView::GetReturnCode() { return returnCode_; };

bool ModalView::IsFinished() { return finished_; };

void ModalView::EndModal(int returnCode) {
  returnCode_ = returnCode;
  finished_ = true;
};

void ModalView::ClearTextRect(int x, int y, int w, int h) {
  View::ClearTextRect(x + left_, y + top_, w, h);
}
void ModalView::DrawString(int x, int y, const char *txt,
                           GUITextProperties &props) {
  View::DrawString(x + left_, y + top_, txt, props);
};

GUIPoint ModalView::GetAnchor() {
  // Get the base anchor point from View
  GUIPoint baseAnchor = View::GetAnchor();

  // Adjust for modal window position
  baseAnchor._x = left_;
  baseAnchor._y = top_;

  return baseAnchor;
}

void ModalView::SetWindow(int width, int height) {

  if (width > 28) {
    width = 28;
  };
  if (height > 20) {
    height = 20;
  };

  left_ = 16 - width / 2;
  top_ = 8 - height / 2;
  if (top_ < 2) {
    top_ = 2;
  }
  ClearTextRect(-1, -1, width + 2, height + 2);

  SetColor(CD_HILITE2);
  GUITextProperties props;

  char line[41];

  // upper border
  memset(line, char_border_single_horizontal, width + 3);
  line[0] = char_border_single_topLeft;
  line[width + 3] = char_border_single_topRight;
  line[width + 4] = 0;
  DrawString(-2, -2, line, props);

  // lower border
  line[0] = char_border_single_bottomLeft;
  line[width + 3] = char_border_single_bottomRight;
  DrawString(-2, height + 1, line, props);

  // left and right borders
  for (int i = 0; i < height + 2; i++) {
    DrawString(-2, i - 1, char_border_single_vertical_s, props);
    DrawString(width + 1, i - 1, char_border_single_vertical_s, props);
  }
};
