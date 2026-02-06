/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _UI_BIG_HEX_VAR_FIELD_H_
#define _UI_BIG_HEX_VAR_FIELD_H_

#include "Foundation/Observable.h"
#include "UIIntVarField.h"

class UIBigHexVarField : public UIIntVarField {

public:
  UIBigHexVarField(const GUIPoint &position, Variable &v, int precision,
                   const char *format, int min, int max, int power,
                   bool wrap = false);
  virtual ~UIBigHexVarField(){};
  virtual void Draw(GUIWindow &w, int offset = 0);
  virtual void ProcessArrow(unsigned short mask);

private:
  unsigned int precision_;
  unsigned int power_;
  unsigned int position_;
  bool wrap_;
};
#endif
