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

// Initialize static instance pointer
ToastView* ToastView::instance_ = nullptr;

// Constructor
ToastView::ToastView(GUIWindow &w, ViewData *viewData) : View(w, viewData) {
  lineCount_ = 0;
  type_ = TT_INFO;
  dismissTime_ = 0;
  visible_ = false;
  
  for (int i = 0; i < 8; i++) {
    lines_[i] = nullptr;
  }
}

ToastView::~ToastView() {
  // Clean up any allocated line pointers
  for (int i = 0; i < lineCount_; i++) {
    if (lines_[i]) {
      delete[] lines_[i];
      lines_[i] = nullptr;
    }
  }
}

void ToastView::UpdateTimer() {
  if (!visible_) {
    return;
  }
  
  if (System::GetInstance()->Millis() >= dismissTime_) {
    visible_ = false;
    Trace::Error("HIDING THE OVCERLATY");
    ((AppWindow &)w_).SetDirty();
  }
}

void ToastView::Show(const char *message, ToastType type, uint32_t timeout_ms) {
  type_ = type;
  
  // Wrap text into lines with padding
  WrapText(message ? message : "");
  
  visible_ = true;
  
  // Set dismiss time
  dismissTime_ = System::GetInstance()->Millis() + timeout_ms;
}

void ToastView::WrapText(const char *message) {
  // Clean up old lines
  for (int i = 0; i < lineCount_; i++) {
    if (lines_[i]) {
      delete[] lines_[i];
      lines_[i] = nullptr;
    }
  }
  lineCount_ = 0;
  
  // Break message into lines that fit within TOAST_MAX_LINE_WIDTH
  int msgLen = strlen(message);
  int start = 0;
  
  while (start < msgLen && lineCount_ < 8) {
    int len = msgLen - start;
    if (len > TOAST_MAX_LINE_WIDTH) {
      len = TOAST_MAX_LINE_WIDTH;
      
      // Try to break at a space
      int breakPos = len;
      for (int i = len - 1; i > len / 2; i--) {
        if (message[start + i] == ' ') {
          breakPos = i;
          break;
        }
      }
      len = breakPos;
    }
    
    // Trim trailing spaces from the content
    int contentLen = len;
    while (contentLen > 0 && message[start + contentLen - 1] == ' ') {
      contentLen--;
    }
    
    // Allocate line with padding: "   content "
    // 3 spaces at start + content + 1 space at end
    int totalLen = 3 + TOAST_MAX_LINE_WIDTH + 1;
    lines_[lineCount_] = new char[totalLen + 1];
    
    // Fill with spaces
    memset(lines_[lineCount_], ' ', totalLen);
    
    // Copy content starting at position 3
    if (contentLen > 0) {
      strncpy(lines_[lineCount_] + 3, message + start, contentLen);
    }
    
    // Null terminate
    lines_[lineCount_][totalLen] = 0;
    
    lineCount_++;
    start += len;
    
    // Skip leading spaces on next line
    while (start < msgLen && message[start] == ' ') {
      start++;
    }
  }

  char icon = GetTypeIcon();
  lines_[0][1] = icon;
}

char ToastView::GetTypeIcon() {
  switch (type_) {
    case TT_INFO: return 'i';
    case TT_ERROR: return 'x';
    case TT_SUCCESS: return 'i';
    case TT_WARNING: return '!';
    default: return '?';
  }
}

void ToastView::Draw(GUIWindow &w, ViewData *viewData) {
  if (!visible_) {
    return;
  }
  
  GUITextProperties props, invprops;
  invprops.invert_ = true;
  SetColor(CD_NORMAL);
  
  // Calculate total height: top border + title lines + divider + message lines + bottom border
  int totalHeight = 3 + lineCount_;
  
  // Position at bottom of screen
  int x = 0;
  int y = SCREEN_HEIGHT - totalHeight;
  
  // Draw top border with corner
  char buffer[SCREEN_WIDTH + 1];
  memset(buffer, ' ', SCREEN_WIDTH);
  buffer[SCREEN_WIDTH] = 0;
  DrawString(x, y++, buffer, invprops);
  
  // Draw title with icon and wrapping
  char icon = GetTypeIcon();
  
  // Draw divider
  const char *border = " ";
  DrawString(0, y, border, invprops);
  DrawString(1, y, buffer, props);
  DrawString(SCREEN_WIDTH - 1, y, border, invprops);
  y++;
  
  // Draw message lines
  for (int i = 0; i < lineCount_; i++) {
    DrawString(0, y, border, invprops);
    DrawString(1, y, lines_[i], props);
    DrawString(SCREEN_WIDTH - 1, y, border, invprops);
    y++;
  }  

  DrawString(0, y, border, invprops);
  DrawString(1, y, buffer, props);
  DrawString(SCREEN_WIDTH - 1, y, border, invprops);
}
