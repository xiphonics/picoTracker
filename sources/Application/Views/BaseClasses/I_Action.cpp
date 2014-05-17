#include "I_Action.h"

I_Action::I_Action(char *name) {
	name_=name ;
} ;

I_Action::~I_Action() {
} ;

char *I_Action::GetName() {
	return name_ ;
} ;
