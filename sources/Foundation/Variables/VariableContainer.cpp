/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "VariableContainer.h"
#include <string.h>

VariableContainer::VariableContainer(Variable **begin, Variable **end)
    : begin_(begin), end_(end){};

VariableContainer::~VariableContainer(){};

Variable *VariableContainer::FindVariable(FourCC id) {
  for (Variable **it = begin_; it != end_; ++it) {
    if ((*it)->GetID() == id) {
      return *it;
    }
  }
  return NULL;
};

Variable *VariableContainer::FindVariable(const char *name) {
  for (Variable **it = begin_; it != end_; ++it) {
    if (!strcmp((*it)->GetName(), name)) {
      return *it;
    }
  }
  return NULL;
};
