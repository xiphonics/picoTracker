/*
 *  SysMutex.h
 *  lgpt
 *
 *  Created by Marc Nostromo on 10/03/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */
#include "pico/mutex.h"

class SysMutex {
public:
  SysMutex();
  ~SysMutex();
  bool Lock();
  void Unlock();

private:
  mutex_t *mutex_;
};

class SysMutexLocker {
public:
  SysMutexLocker(SysMutex &mutex);
  ~SysMutexLocker();

private:
  SysMutex *mutex_;
};
