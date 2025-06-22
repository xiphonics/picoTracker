/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _GROOVE_VIEW_H_
#define _GROOVE_VIEW_H_

#include "ScreenView.h"
#include "ViewData.h"

class GrooveView : public ScreenView {
public:
  GrooveView(GUIWindow &w, ViewData *viewData);
  ~GrooveView();
  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int tick = 0);
  virtual void OnFocus();

protected:
  void updateCursorValue(int val, bool sync = false);
  void updateCursor(int dir);
  void initCursorValue();
  void clearCursorValue();
  void warpGroove(int dir);
  void processNormalButtonMask(unsigned short mask);
  void processSelectionButtonMask(unsigned short mask);

private:
  int position_;
  int lastPosition_;
};
#endif