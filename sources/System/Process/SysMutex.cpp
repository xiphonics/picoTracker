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

SysMutex::~SysMutex() {}

bool SysMutex::Lock() {
  if (!mutex_) {
    mutex_init(mutex_);
  }
  if (mutex_) {
    mutex_enter_blocking(mutex_);
    return true;
  }
  return false;
}

void SysMutex::Unlock() {
  if (mutex_) {
    mutex_exit(mutex_);
  }
}

SysMutexLocker::SysMutexLocker(SysMutex &mutex) : mutex_(&mutex) {
  mutex_->Lock();
}

SysMutexLocker::~SysMutexLocker() { mutex_->Unlock(); }
