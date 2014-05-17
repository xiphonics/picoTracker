#ifndef _Unix_PROCESS_H_
#define _Unix_PROCESS_H_

#include "System/Process/Process.h"
#include <pthread.h>
#include <semaphore.h>

class UnixProcessFactory:public SysProcessFactory {
	bool BeginThread(SysThread &) ;
	virtual SysSemaphore *CreateNewSemaphore(int initialcount = 0, int maxcount = 0) ;
} ;

class UnixSysSemaphore:public SysSemaphore {
public:
	UnixSysSemaphore(int initialcount = 0, int maxcount = 0) ;
	virtual ~UnixSysSemaphore() ;
	virtual SysSemaphoreResult Wait() ;
	virtual SysSemaphoreResult TryWait() ;
	virtual SysSemaphoreResult WaitTimeout(unsigned long) ;
	virtual SysSemaphoreResult Post() ;
private:
	sem_t *sem_ ;
} ;
#endif
