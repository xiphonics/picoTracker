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

#include "Externals/etl/include/etl/list.h"
#include "Variable.h"

class VariableContainer {
public:
  VariableContainer(etl::ilist<Variable *> *list);
  virtual ~VariableContainer();
  Variable *FindVariable(FourCC id);
  Variable *FindVariable(const char *name);

private:
  etl::ilist<Variable *> *list_;
};
#endif
