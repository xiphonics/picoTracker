#ifndef _W32_PROCESS_H_
#define _W32_PROCESS_H_

#include "System/Process/Process.h"
#include <windows.h>

class W32ProcessFactory:public SysProcessFactory {
	bool BeginThread(SysThread &) ;
	virtual SysSemaphore *CreateNewSemaphore(int initialcount = 0, int maxcount = 0) ;
} ;

class W32SysSemaphore:public SysSemaphore {
public:
	W32SysSemaphore(int initialcount = 0, int maxcount = 0) ;
	virtual ~W32SysSemaphore() ;
	virtual SysSemaphoreResult Wait() ;
	virtual SysSemaphoreResult TryWait() ;
	virtual SysSemaphoreResult WaitTimeout(unsigned long) ;
	virtual SysSemaphoreResult Post() ;
private:
	HANDLE handle_ ;
} ;
#endif