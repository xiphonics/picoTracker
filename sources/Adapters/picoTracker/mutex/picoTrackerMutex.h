#ifndef _PICOTRACKER_MUTEX_H_
#define _PICOTRACKER_MUTEX_H_

#include "System/Process/SysMutex.h"
#include "pico/mutex.h"

class picoTrackerMutex : public SysMutex {
public:
  picoTrackerMutex();
  virtual ~picoTrackerMutex(){};
  virtual bool Lock() override;
  virtual void Unlock() override;

private:
  mutex_t mutex_;
};

#endif // _PICOTRACKER_MUTEX_H_
