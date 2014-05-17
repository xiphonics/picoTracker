
#include "Service.h"
#include "ServiceRegistry.h"

Service::Service(int fourCC) {
	fourCC_=fourCC ;
	ServiceRegistry::GetInstance()->Register(this) ;
} ;

Service::~Service() {
} ;

void Service::Register(SubService *sub) {
	Insert(sub) ;
} ;

void Service::Unregister(SubService *sub) {
	Remove(*sub) ;
} ;

