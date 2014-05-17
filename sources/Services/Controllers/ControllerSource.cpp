
#include "ControllerSource.h"
#include "ControllerService.h"
//#include "System/Memory/Memory.h"
#include <string.h>

ControllerSource::ControllerSource(const char *devclass,const char *name) {

	name_=name;
	class_=devclass ;
	ControllerService::GetInstance()->Insert(*this) ;
} ;

ControllerSource::~ControllerSource() {
	ControllerService::GetInstance()->Remove(*this) ;
} ;

const char *ControllerSource::GetClass() {
	return class_.c_str() ;
} ;

const char *ControllerSource::GetName() {
	return name_.c_str() ;
} ;