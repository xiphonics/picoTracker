#ifndef _PERSISTENCY_SERVICE_H_
#define _PERSISTENCY_SERVICE_H_

#include "Externals/TinyXML/tinyxml.h"
#include "Externals/yxml/yxml.h"
#include "Foundation/Services/Service.h"
#include "Foundation/T_Singleton.h"

class PersistencyService: public Service,public T_Singleton<PersistencyService> {
public:
	PersistencyService() ;
	void Save() ;
	bool Load() ;
} ;

#endif
