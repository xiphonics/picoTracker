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

VariableContainer::VariableContainer(etl::ilist<Variable *> *list)
    : list_(list){};

VariableContainer::~VariableContainer(){};

Variable *VariableContainer::FindVariable(FourCC id) {
  auto it = list_->begin();
  for (size_t i = 0; i < list_->size(); i++) {
    if ((*it)->GetID() == id) {
      return *it;
    }
    it++;
  }
  return NULL;
};

Variable *VariableContainer::FindVariable(const char *name) {
  auto it = list_->begin();
  for (size_t i = 0; i < list_->size(); i++) {
    if (!strcmp((*it)->GetName(), name)) {
      return *it;
    }
    it++;
  }
  return NULL;
};
