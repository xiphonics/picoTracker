#ifndef _VARIABLE_CONTAINER_H_
#define _VARIABLE_CONTAINER_H_

#include "Externals/etl/include/etl/list.h"
#include "Foundation/T_SimpleList.h"
#include "Variable.h"

class VariableContainer : public etl::list<Variable *, 30> {
public:
  VariableContainer();
  virtual ~VariableContainer();
  Variable *FindVariable(FourCC id);
  Variable *FindVariable(const char *name);
};
#endif
