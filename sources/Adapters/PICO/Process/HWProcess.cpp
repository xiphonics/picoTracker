
#include "HWProcess.h"
#include "System/Console/Trace.h"
#include <fcntl.h>
#include <stdio.h>
// #include "Externals/FreeRTOS/FreeRTOS-Kernel/include/FreeRTOS.h"
// #include "Externals/FreeRTOS/FreeRTOS-Kernel/include/task.h"

void _HWStartThread(void *p) {
  SysThread *play = (SysThread *)p;
  play->startExecution();
  // TODO: cleanup
  //	return NULL ;
}

bool HWProcessFactory::BeginThread(SysThread &thread) {
  printf("Created new thread...\n");
  // TODO: see what to do with threads. If only one then we can use
  // https://raspberrypi.github.io/pico-sdk-doxygen/group__pico__multicore.html
  //	pthread_create(&pthread,0,_HWStartThread,&thread) ;
  // TODO: reenable threads when we reach here
  //  TaskHandle_t pthread;
  //  xTaskCreate(_HWStartThread,"name",256,&thread,1,&pthread);
  return true;
}

SysSemaphore *HWProcessFactory::CreateNewSemaphore(int initialcount,
                                                   int maxcount) {
  return new HWSysSemaphore(initialcount, maxcount);
};

HWSysSemaphore::HWSysSemaphore(int initialcount, int maxcount) {
  sem_init(sem_, initialcount, maxcount);
};

HWSysSemaphore::~HWSysSemaphore() {
  // Should release? There is no equivalent to destroying the semaphore
  // (sem_unlink)
  sem_release(sem_);
};

SysSemaphoreResult HWSysSemaphore::Wait() {
  sem_acquire_blocking(sem_);
  return SSR_NO_ERROR;
};

SysSemaphoreResult HWSysSemaphore::TryWait() { return SSR_INVALID; }

SysSemaphoreResult HWSysSemaphore::WaitTimeout(unsigned long timeout) {
  return SSR_INVALID;
};

SysSemaphoreResult HWSysSemaphore::Post() {
  sem_release(sem_);
  return SSR_NO_ERROR;
};
