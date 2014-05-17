
#ifndef _SERVICE_REGISTRY_H_
#define _SERVICE_REGISTRY_H_

#include "Foundation/T_Singleton.h"
#include "Service.h"
#include "SubService.h"

class ServiceRegistry: public T_Singleton<ServiceRegistry> {
public:
	void Register(Service *) ;
	void Register(SubService *) ;
	void Unregister(SubService *) ;
protected:
    T_SimpleList<Service> services_ ;
} ;
#endif
