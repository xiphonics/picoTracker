/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _UI_SWATCH_FIELD_H_
#define _UI_SWATCH_FIELD_H_

#include "UIField.h"

class UISwatchField : public UIField {

public:
  UISwatchField(GUIPoint &position, const ColorDefinition color);
  virtual ~UISwatchField(){};
  virtual void Draw(GUIWindow &w, int offset = 0);
  virtual void ProcessArrow(unsigned short mask);
  virtual void OnClick(){};

  virtual bool IsStatic();

protected:
protected:
  ColorDefinition color_;
};

#endif
