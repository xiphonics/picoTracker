/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2025 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "ToastView.h"
#include "Application/AppWindow.h"
#include <string.h>

#define TOAST_MAX_LINE_WIDTH (SCREEN_WIDTH - 6)

ToastView *ToastView::instance_ = nullptr;

ToastView::ToastView(GUIWindow &w, ViewData *viewData) : View(w, viewData){};
ToastView::~ToastView() { DeleteStrings(); }
ToastView *ToastView::getInstance() { return instance_; }

void ToastView::Init(GUIWindow &w, ViewData *viewData) {
  if (!instance_)
    instance_ = new ToastView(w, viewData);
}

// clean up allocated strings
void ToastView::DeleteStrings() {
  for (int i = 0; i < lineCount_; i++) {
    delete[] lines_[i];
    lines_[i] = nullptr;
  }
  lineCount_ = 0;
}

// splits the message into multiple lines fitting into the toast width and pads
// with spaces on both sides
void ToastView::WrapText(const char *message) {
  DeleteStrings();

  const int totalLen = 3 + maxLineWidth + 1;

  // helper lambda to add an empty line (top and bottom)
  auto addEmptyLine = [&]() {
    lines_[lineCount_] = new char[totalLen + 1];
    memset(lines_[lineCount_], ' ', totalLen);
    lines_[lineCount_++][totalLen] = 0;
  };

  addEmptyLine(); // empty line at start

  int msgLen = strlen(message);
  int pos = 0;

  while (pos < msgLen && lineCount_ < maxLines - 1) {
    int remaining = msgLen - pos;
    int lineLen = remaining > maxLineWidth ? maxLineWidth : remaining;

    // try to break at last space after half the width, break mid-word if needed
    if (lineLen == maxLineWidth) {
      for (int i = lineLen - 1; i >= lineLen / 2; i--) {
        if (message[pos + i] == ' ') {
          lineLen = i;
          break;
        }
      }
    }

    // allocate and populate next line
    lines_[lineCount_] = new char[totalLen + 1];
    memset(lines_[lineCount_], ' ', totalLen);
    if (lineLen > 0)
      memcpy(lines_[lineCount_] + 3, message + pos, lineLen);
    lines_[lineCount_++][totalLen] = 0;

    // skip to the next line
    pos += lineLen;

    // trim, leadin spaces from wrapping
    while (pos < msgLen && message[pos] == ' ')
      pos++;
  }

  addEmptyLine(); // empty line at end
}

void ToastView::UpdateTimer() {
  if (!visible_)
    return;

  uint32_t now = System::GetInstance()->Millis();

  // check if we should start animating out
  if (now >= dismissTime_ && animationStartTime_ == 0) {
    animationStartTime_ = now;
    ((AppWindow &)w_).SetDirty();
  }

  // update animation offset
  if (animationStartTime_ > 0) {
    int newOffset = (now - animationStartTime_) / 50; // one row per 50ms

    if (newOffset >= 3 + lineCount_) {
      // animation complete, hide the toast
      visible_ = false;
      animationOffset_ = 0;
      animationStartTime_ = 0;
      ((AppWindow &)w_).SetDirty();
    } else if (newOffset != animationOffset_) {
      // animation in progress, update offset
      animationOffset_ = newOffset;
      ((AppWindow &)w_).SetDirty();
    }
  }
}

void ToastView::Show(const char *text, const ToastType *type, uint32_t msTime) {
  type_ = *type;
  visible_ = true;
  animationOffset_ = 0;
  animationStartTime_ = 0;
  dismissTime_ = System::GetInstance()->Millis() + msTime;
  WrapText(text);
}

void ToastView::Draw(GUIWindow &w, ViewData *viewData) {
  if (!visible_)
    return;

  GUITextProperties props, invprops;
  invprops.invert_ = true;
  SetColor(CD_NORMAL);

  int y = std::max(0, (int)(SCREEN_HEIGHT - lineCount_ - 1 + animationOffset_));
  int iconY = y + 2;

  char buffer[SCREEN_WIDTH + 1];
  memset(buffer, ' ', SCREEN_WIDTH);
  buffer[SCREEN_WIDTH] = 0;

  // top border
  if (y < SCREEN_HEIGHT)
    DrawString(0, y++, buffer, invprops);

  // message lines
  for (int i = 0; i < lineCount_ && y < SCREEN_HEIGHT; i++, y++) {
    DrawString(0, y, " ", invprops);
    DrawString(1, y, lines_[i], props);
    DrawString(SCREEN_WIDTH - 1, y, " ", invprops);
  }

  // add the icon
  SetColor(type_.color);
  if (iconY < SCREEN_HEIGHT)
    DrawString(2, iconY, type_.symbol, props);
}