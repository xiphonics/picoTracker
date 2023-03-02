/*
 *  SysMutex.cpp
 *  lgpt
 *
 *  Created by Marc Nostromo on 10/03/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "SysMutex.h"

SysMutex::SysMutex() : mutex_(0) {}

SysMutex::~SysMutex() {
#ifndef PICOBUILD
  if (mutex_) {
    SDL_DestroyMutex(mutex_);
    mutex_ = NULL;
  }
#endif
}

bool SysMutex::Lock() {
#ifndef PICOBUILD
  if (!mutex_) {
    mutex_ = SDL_CreateMutex();
  }
  if (mutex_) {
    SDL_LockMutex(mutex_);
    return true;
  }
#else
  if (!mutex_) {
    mutex_init(mutex_);
  }
  if (mutex_) {
    mutex_enter_blocking(mutex_);
    return true;
  }
#endif
  return false;
}

void SysMutex::Unlock() {
  if (mutex_) {
#ifndef PICOBUILD
    SDL_UnlockMutex(mutex_);
#else
    mutex_exit(mutex_);
#endif
  }
}

SysMutexLocker::SysMutexLocker(SysMutex &mutex) : mutex_(&mutex) {
  mutex_->Lock();
}

SysMutexLocker::~SysMutexLocker() { mutex_->Unlock(); }
