#include "picoTrackerMutex.h"

picoTrackerMutex::picoTrackerMutex() { mutex_init(&mutex_); }

inline bool picoTrackerMutex::Lock() {
  mutex_enter_blocking(&mutex_);
  return true;
}

inline void picoTrackerMutex::Unlock() { mutex_exit(&mutex_); }
