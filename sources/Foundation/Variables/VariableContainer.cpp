#include "VariableContainer.h"
#include <string.h>

VariableContainer::VariableContainer(){};

VariableContainer::~VariableContainer(){};

Variable *VariableContainer::FindVariable(FourCC id) {
  auto it = begin();
  for (size_t i = 0; i < size(); i++) {
    if ((*it)->GetID() == id) {
      return *it;
    }
    it++;
  }
  return NULL;
};

Variable *VariableContainer::FindVariable(const char *name) {
  auto it = begin();
  for (size_t i = 0; i < size(); i++) {
    if (!strcmp((*it)->GetName(), name)) {
      return *it;
    }
    it++;
  }
  return NULL;
};
