#ifndef _TIME_SERVICE_H_
#define _TIME_SERVICE_H_

#include "Foundation/T_Singleton.h"
#include <string>

typedef double Time ;

class TimeService: public T_Singleton<TimeService> {
public:
	TimeService() ;
	virtual ~TimeService() ;
	Time GetTime() ;
	void Sleep(int msecs) ;
private:
	unsigned long startTick_ ;
} ;
#endif
