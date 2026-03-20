/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _UI_INT_FIELD_H_
#define _UI_INT_FIELD_H_

#include "UIField.h"

class UIIntField : public UIField {

public:
  UIIntField(const GUIPoint &position, int *src, const char *format, int min,
             int max, int xOffset, int yOffset);
  virtual ~UIIntField(){};
  virtual void Draw(GUIWindow &w, int offset = 0) override;
  virtual void ProcessArrow(unsigned short mask);
  virtual void OnClick(){};

protected:
protected:
  int *src_;
  const char *format_;
  int min_;
  int max_;
  int xOffset_;
  int yOffset_;
};

#endif
