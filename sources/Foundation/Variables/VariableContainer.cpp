#include "VariableContainer.h"
#include <string.h>

VariableContainer::VariableContainer():T_SimpleList<Variable>(true) {
};

VariableContainer::~VariableContainer() {
};

Variable *VariableContainer::FindVariable(FourCC id) {
	IteratorPtr<Variable> it(GetIterator()) ;
	for (it->Begin();!it->IsDone();it->Next()) {
		Variable &v=it->CurrentItem() ;
		if (v.GetID()==id) {
			return &v ;
		}; 
	};
	return NULL ;
} ;

Variable *VariableContainer::FindVariable(const char *name) {
	IteratorPtr<Variable> it(GetIterator()) ;
	for (it->Begin();!it->IsDone();it->Next()) {
		Variable &v=it->CurrentItem() ;
		if (!strcmp(v.GetName(),name)) {
			return &v ;
		}; 
	};
	return NULL ;
} ;
