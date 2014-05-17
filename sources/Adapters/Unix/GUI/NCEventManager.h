/*
 *  NCEventManager.h
 *  lgpt
 *
 *  Created by Marc Nostromo on 24/03/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _NC_EVENT_MANAGER_
#define _NC_EVENT_MANAGER_

#include "Foundation/T_Singleton.h"
#include "UIFramework/SimpleBaseClasses/EventManager.h"

#include <string>



class NCEventManager: public T_Singleton<NCEventManager>,public EventManager {
public:
	NCEventManager() ;
	~NCEventManager() ;
	virtual bool Init() ;
	virtual int MainLoop() ;
	virtual void PostQuitMessage() ;
	virtual int GetKeyCode(const char *name) ;
	
private:
} ;
#endif
