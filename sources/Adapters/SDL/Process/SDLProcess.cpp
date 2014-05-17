
#include "SDLProcess.h"
#include <SDL/SDL_thread.h>

int _SDLStartThread(void *argp) {
	SysThread *play=(SysThread *)argp ;
	play->startExecution() ;
	return 0 ;
}

bool SDLProcessFactory::BeginThread(SysThread& thread) {
	SDL_CreateThread(_SDLStartThread,&thread);
	return true ;
}

SysSemaphore *SDLProcessFactory::CreateNewSemaphore(int initialcount, int maxcount) {
	return new SDLSysSemaphore(initialcount,maxcount) ;
} ;

SDLSysSemaphore::SDLSysSemaphore(int initialcount,int maxcount) {
	handle_=SDL_CreateSemaphore(0) ;
} ;

SDLSysSemaphore::~SDLSysSemaphore() {
	handle_=0 ;
} ;

SysSemaphoreResult SDLSysSemaphore::Wait() {
	if (!handle_) {
		return SSR_INVALID ;
	} ;
	return (SysSemaphoreResult)SDL_SemWait(handle_) ;
} ;

SysSemaphoreResult SDLSysSemaphore::TryWait() {
	if (!handle_) {
		return SSR_INVALID ;
	} ;
	return (SysSemaphoreResult)0;
}

SysSemaphoreResult SDLSysSemaphore::WaitTimeout(unsigned long timeout) {
	if (!handle_) {
		return SSR_INVALID ;
	} ;
	return (SysSemaphoreResult)0;
} ;

SysSemaphoreResult SDLSysSemaphore::Post() {
	if (!handle_) {
		return SSR_INVALID ;
	} ;
	return (SysSemaphoreResult)SDL_SemPost(handle_) ;
} ;
