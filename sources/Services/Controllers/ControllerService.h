
#ifndef _CONTROLLER_SERVICE_H_
#define _CONTROLLER_SERVICE_H_

#include "Foundation/T_Singleton.h"
#include "Foundation/T_SimpleList.h"

#include "ControllerSource.h"
#include "ControllableVariable.h"
#include "Services/Time/TimeService.h"

class ControllerService: public T_Singleton<ControllerService>,public T_SimpleList<ControllerSource> {
public:
	ControllerService();
	virtual ~ControllerService();
	Channel *GetChannel(const char *sourcePath) ;
} ;
#endif
