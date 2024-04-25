/*
 *  SysMutex.h
 *  lgpt
 *
 *  Created by Marc Nostromo on 10/03/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef PICOBUILD
#include <SDL2/SDL.h>
#else
#include "pico/mutex.h"
#endif

class SysMutex {
public:
  SysMutex();
  ~SysMutex();
  bool Lock();
  void Unlock();

private:
#ifndef PICOBUILD
  SDL_mutex *mutex_;
#else
  mutex_t *mutex_;
#endif
};

class SysMutexLocker {
public:
  SysMutexLocker(SysMutex &mutex);
  ~SysMutexLocker();

private:
  SysMutex *mutex_;
};
