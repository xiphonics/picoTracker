/*
 *  SysMutex.cpp
 *  lgpt
 *
 *  Created by Marc Nostromo on 10/03/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "SysMutex.h"

SysMutexLocker::SysMutexLocker(SysMutex &mutex) : mutex_(&mutex) {
  mutex_->Lock();
}

SysMutexLocker::~SysMutexLocker() { mutex_->Unlock(); }
