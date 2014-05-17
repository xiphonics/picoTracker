
#include "RTTypeInfo.h"

// Constructor

RTTypeInfo::RTTypeInfo() {
	_registered=false ;
}

// Returns if the type info has been registered

bool RTTypeInfo::IsRegistered() {
	return _registered ;
}

// Sets the registration flag

void RTTypeInfo::SetRegistered() {
	_registered=true ;
}
