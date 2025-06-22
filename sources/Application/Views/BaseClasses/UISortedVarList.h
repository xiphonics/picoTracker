/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _UI_SORTED_VAR_LIST_H_
#define _UI_SORTED_VAR_LIST_H_

#include "UIIntVarField.h"

class UISortedVarList : public UIIntVarField {

public:
  UISortedVarList(GUIPoint &position, Variable &v, const char *format);
  virtual ~UISortedVarList(){};
  virtual void ProcessArrow(unsigned short mask);
};

#endif
