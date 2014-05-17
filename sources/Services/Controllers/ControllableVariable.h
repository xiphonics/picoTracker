
#ifndef _CONTROLLABLE_VARIABLE_H_
#define _CONTROLLABLE_VARIABLE_H_

#include "Foundation/Observable.h"
#include "Foundation/Variables/WatchedVariable.h"
#include "Channel.h"

class ControllableVariable:public WatchedVariable,I_Observer {
public:
	bool Connect(Channel &channel) ;
	void Disconnect() ;
} ;
#endif
