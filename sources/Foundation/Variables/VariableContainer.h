/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _VARIABLE_CONTAINER_H_
#define _VARIABLE_CONTAINER_H_

#include "Variable.h"

class VariableContainer {
public:
  VariableContainer(Variable **begin, Variable **end);
  virtual ~VariableContainer();
  Variable *FindVariable(FourCC id);
  Variable *FindVariable(const char *name);
  Variable **VarBegin() const { return begin_; }
  Variable **VarEnd() const { return end_; }

private:
  Variable **begin_;
  Variable **end_;
};
#endif
