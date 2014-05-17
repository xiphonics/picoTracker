/*
 *  SysMutex.h
 *  lgpt
 *
 *  Created by Marc Nostromo on 10/03/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include <SDL/SDL.h>

class SysMutex {
public:
	SysMutex() ;
	~SysMutex() ;
	bool Lock() ;
	void Unlock() ;
private:
	SDL_mutex *mutex_ ;
} ;

class SysMutexLocker {
public:
	SysMutexLocker(SysMutex &mutex) ;
	~SysMutexLocker() ;
private:
	SysMutex *mutex_ ;
} ;