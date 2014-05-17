#ifndef _VARIABLE_CONTAINER_H_
#define _VARIABLE_CONTAINER_H_

#include "Variable.h"
#include "Foundation/T_SimpleList.h"

class VariableContainer: public T_SimpleList<Variable> {
public:
	VariableContainer() ;
	virtual ~VariableContainer() ;
	Variable *FindVariable(FourCC id) ;
	Variable *FindVariable(const char *name) ;
} ;
#endif
