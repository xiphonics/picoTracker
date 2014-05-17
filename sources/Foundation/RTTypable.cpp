

#include "RTTypable.h"
#include <iostream.h>

// Constructor

RTTypable::RTTypable() {
}


// Destructor

RTTypable::~RTTypable() {
}

// Registration routine. All instances of RTTypeInfo created
// should call this

void RTTypable::Register(RTTypeInfo &info) {
	info.SetRegistered() ;
}


