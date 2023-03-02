#ifndef _HW_PROCESS_H_
#define _HW_PROCESS_H_

#include "System/Process/Process.h"
// #include <pthread.h>
#include "pico/sem.h"

class HWProcessFactory : public SysProcessFactory {
  bool BeginThread(SysThread &);
  virtual SysSemaphore *CreateNewSemaphore(int initialcount = 0,
                                           int maxcount = 0);
};

class HWSysSemaphore : public SysSemaphore {
public:
  HWSysSemaphore(int initialcount = 0, int maxcount = 0);
  virtual ~HWSysSemaphore();
  virtual SysSemaphoreResult Wait();
  virtual SysSemaphoreResult TryWait();
  virtual SysSemaphoreResult WaitTimeout(unsigned long);
  virtual SysSemaphoreResult Post();

private:
  semaphore_t *sem_;
};
#endif
