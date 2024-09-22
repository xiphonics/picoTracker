#ifndef _VARIABLE_CONTAINER_H_
#define _VARIABLE_CONTAINER_H_

#include "Externals/etl/include/etl/list.h"
#include "Foundation/T_SimpleList.h"
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
