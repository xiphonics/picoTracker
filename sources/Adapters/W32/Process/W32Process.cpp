
#include "W32Process.h"
#include <process.h>

void _W32StartThread(void *p) {
	SysThread *play=(SysThread *)p ;
	play->startExecution() ;
}

bool W32ProcessFactory::BeginThread(SysThread& thread) {
	_beginthread(_W32StartThread,0,&thread) ;
	return true ;
}

SysSemaphore *W32ProcessFactory::CreateNewSemaphore(int initialcount, int maxcount) {
	return new W32SysSemaphore(initialcount,maxcount) ;
} ;

W32SysSemaphore::W32SysSemaphore(int initialcount,int maxcount) {
	handle_=CreateSemaphore(NULL,initialcount,maxcount,NULL) ;
} ;

W32SysSemaphore::~W32SysSemaphore() {
	CloseHandle(handle_) ;
} ;

SysSemaphoreResult W32SysSemaphore::Wait() {
	if (!handle_) {
		return SSR_INVALID ;
	} ;
	return WaitTimeout(INFINITE) ;
} ;

SysSemaphoreResult W32SysSemaphore::TryWait() {
	if (!handle_) {
		return SSR_INVALID ;
	} ;
	return WaitTimeout(0) ;
}

SysSemaphoreResult W32SysSemaphore::WaitTimeout(unsigned long timeout) {
	if (!handle_) {
		return SSR_INVALID ;
	} ;
	DWORD result=WaitForSingleObject(handle_,timeout) ;
	switch (result) {
		case WAIT_OBJECT_0:
			return SSR_NO_ERROR ;
			break ;
		case WAIT_TIMEOUT:
			return SSR_BUSY ;
			break ;
		default:
			return SSR_OTHER_ERROR ;
	} ;
} ;

SysSemaphoreResult W32SysSemaphore::Post() {
	if (!handle_) {
		return SSR_INVALID ;
	} ;
	if (!ReleaseSemaphore(handle_,1,NULL)) {
		return SSR_OVERFLOW ; 
	};
	return SSR_NO_ERROR ;
} ;
