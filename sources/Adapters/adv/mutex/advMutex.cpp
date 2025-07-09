#include "advMutex.h"
#include "System/Console/Trace.h"

advMutex::advMutex() : mutex_(xSemaphoreCreateBinary()) {
  // Semaphore is created in an empty state
  xSemaphoreGive(mutex_);
}

bool advMutex::Lock() {
  if (xSemaphoreTake(mutex_, portMAX_DELAY)) {
    return true;
  }
  return false;
}

void advMutex::Unlock() {
  if (!xSemaphoreGive(mutex_)) {
    Trace::Error("Semaphore release error");
  }
}
