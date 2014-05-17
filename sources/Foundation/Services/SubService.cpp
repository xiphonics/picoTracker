
#include "SubService.h"
#include "ServiceRegistry.h"

SubService::SubService(int fourCC) {
	fourCC_=fourCC ;
	ServiceRegistry::GetInstance()->Register(this) ;
} ;

SubService::~SubService() {
	ServiceRegistry::GetInstance()->Unregister(this) ;
} ;
