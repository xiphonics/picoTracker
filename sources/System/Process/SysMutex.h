/*
 *  SysMutex.h
 *  lgpt
 *
 *  Created by Marc Nostromo on 10/03/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _SYS_MUTEX_H_
#define _SYS_MUTEX_H_

class SysMutex {
public:
  SysMutex(){};
  ~SysMutex(){};
  virtual bool Lock() = 0;
  virtual void Unlock() = 0;
};

class SysMutexLocker {
public:
  SysMutexLocker(SysMutex &mutex);
  ~SysMutexLocker();

private:
  SysMutex *mutex_;
};

#endif
