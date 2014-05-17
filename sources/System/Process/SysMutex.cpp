/*
 *  SysMutex.cpp
 *  lgpt
 *
 *  Created by Marc Nostromo on 10/03/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "SysMutex.h"

SysMutex::SysMutex()
:mutex_(0)
{
}

SysMutex::~SysMutex() {
	if (mutex_) {
		SDL_DestroyMutex(mutex_) ;
		mutex_ = NULL ;
	}
}

bool SysMutex::Lock() {
	if (!mutex_) {
		mutex_ = SDL_CreateMutex() ;
	}
	if (mutex_) {
		SDL_LockMutex(mutex_) ;
		return true ;
	} 
	return false ;
}

void SysMutex::Unlock() {
	if (mutex_) {
		SDL_UnlockMutex(mutex_) ;
	}
}

SysMutexLocker::SysMutexLocker(SysMutex &mutex)
:mutex_(&mutex)
{
	mutex_->Lock() ;
}

SysMutexLocker::~SysMutexLocker() {
	mutex_->Unlock() ;
}