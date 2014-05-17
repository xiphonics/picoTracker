#ifndef _NOS_PROCESS_H_
#define _NOS_PROCESS_H_

#include "Foundation/T_Factory.h"

class SysThreadExecuter ;

// Base class for threads

class SysThread {

public:
	SysThread() { isFinished_=false ; shouldTerminate_=false ; } ;
	virtual ~SysThread() {} ;
	bool Start() ;
	virtual void RequestTermination() ;
	bool IsFinished() ;

public: // Override in subclasses
	virtual bool Execute()=0 ;
public:
	bool startExecution() ; 
protected:
	bool shouldTerminate() ;

private:
	bool shouldTerminate_ ;
	bool isFinished_ ;
} ;

// semaphores

enum  SysSemaphoreResult {
	SSR_NO_ERROR,
	SSR_INVALID,
	SSR_BUSY,
	SSR_OVERFLOW,
	SSR_OTHER_ERROR
}  ;

class SysSemaphore {

public:
	SysSemaphore() {} ;
	virtual ~SysSemaphore() {} ;
	virtual SysSemaphoreResult Wait()=0 ;
	virtual SysSemaphoreResult TryWait()=0 ;
	virtual SysSemaphoreResult WaitTimeout(unsigned long)=0 ;
	virtual SysSemaphoreResult Post()=0 ;
public:
	static SysSemaphore *Create(int initialcount = 0, int maxcount = 0) ;
} ;

// The thread executer

class SysProcessFactory: public T_Factory<SysProcessFactory> {
public:
	virtual bool BeginThread(SysThread &)=0 ;
	virtual SysSemaphore *CreateNewSemaphore(int initialcount = 0, int maxcount = 0)=0 ;

} ;


#endif
