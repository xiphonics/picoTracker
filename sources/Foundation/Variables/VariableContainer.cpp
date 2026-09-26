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

VariableContainer::VariableContainer(etl::array_view<Variable *> list)
    : list_(list){};

VariableContainer::~VariableContainer(){};

Variable *VariableContainer::FindVariable(FourCC id) {
  for (auto it = list_.begin(); it != list_.end(); it++) {
    if (*it && (*it)->GetID() == id) {
      return *it;
    }
  }
  return NULL;
};

Variable *VariableContainer::FindVariable(const char *name) {
  for (auto it = list_.begin(); it != list_.end(); it++) {
    if (*it && !strcmp((*it)->GetName(), name)) {
      return *it;
    }
  }
  return NULL;
};
