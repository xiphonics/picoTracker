/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _UI_FIELD_H_
#define _UI_FIELD_H_

#include "System/Console/Trace.h"
#include "UIFramework/BasicDatas/GUIPoint.h"
#include "UIFramework/SimpleBaseClasses/GUIWindow.h"
#include "View.h"

class UIField {
public:
  UIField(GUIPoint &position);
  virtual ~UIField();
  virtual void Draw(GUIWindow &w, int offset = 0) = 0;
  virtual void OnClick() = 0; // ENTER pressed
  virtual void ProcessArrow(unsigned short mask) = 0;
  virtual void OnEditClick(){}; // EDIT pressed
  virtual void ProcessEditArrow(unsigned short mask){};
  void SetFocus();
  void ClearFocus();
  bool HasFocus();
  void SetPosition(GUIPoint &);
  GUIPoint GetPosition();
  GUIColor GetColor();

  virtual bool IsStatic();

protected:
  uint8_t x_;
  uint8_t y_;
  bool focus_;
};
#endif
