#ifndef _SDL_PROCESS_H_
#define _SDL_PROCESS_H_

#include "System/Process/Process.h"
#include <SDL/SDL.h>

class SDLProcessFactory:public SysProcessFactory {
	bool BeginThread(SysThread &) ;
	virtual SysSemaphore *CreateNewSemaphore(int initialcount = 0, int maxcount = 0) ;
} ;

class SDLSysSemaphore:public SysSemaphore {
public:
	SDLSysSemaphore(int initialcount = 0, int maxcount = 0) ;
	virtual ~SDLSysSemaphore() ;
	virtual SysSemaphoreResult Wait() ;
	virtual SysSemaphoreResult TryWait() ;
	virtual SysSemaphoreResult WaitTimeout(unsigned long) ;
	virtual SysSemaphoreResult Post() ;
private:
	SDL_sem *handle_ ;
} ;
#endif
