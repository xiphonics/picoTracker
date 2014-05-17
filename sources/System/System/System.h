#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "Foundation/T_Factory.h"
#include "typedefs.h"
#include <stdlib.h>


class System: public T_Factory<System> {

public: // Override in implementation
	virtual unsigned long GetClock()=0 ; // millisecs
	virtual int GetBatteryLevel()=0 ;
	virtual void *Malloc(unsigned size)=0 ;
	virtual void Free(void *)=0 ;
  virtual void Memset(void *addr,char value,int size)=0 ;
  virtual void *Memcpy(void *s1, const void *s2, int n)=0  ;
	virtual void PostQuitMessage()=0 ;
	virtual unsigned int GetMemoryUsage()=0 ;

} ;

#define SYS_MEMSET(a,b,c) { System* system=System::GetInstance() ; system->Memset(a,b,c) ;  }
#define SYS_MEMCPY(a,b,c) {  System* system=System::GetInstance() ; system->Memcpy(a,b,c) ; }
#define SYS_MALLOC(size) (System::GetInstance()->Malloc(size))
#define SYS_FREE(ptr) (System::GetInstance()->Free(ptr))

#define SAFE_DELETE(ptr) if (ptr)  { delete ptr ; ptr=0 ; }
#define SAFE_FREE(ptr) if (ptr) { SYS_FREE(ptr); ptr=0 ; }

#endif
