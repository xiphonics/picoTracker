
#ifndef _STATUS_H_
#define _STATUS_H_

#include "Foundation/T_Factory.h"

class Status: public T_Factory<Status>  {
public:
	virtual void Print(char *)=0 ;
	static void Set(char *fmt, ...) ;
} ;

#endif
