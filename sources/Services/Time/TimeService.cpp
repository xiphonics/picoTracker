#include "TimeService.h"
#include "System/System/System.h"
#include "pico/stdlib.h"

TimeService::TimeService() { startTick_ = System::GetInstance()->GetClock(); };

TimeService::~TimeService(){};

Time TimeService::GetTime() {
  unsigned long currentTick = System::GetInstance()->GetClock();
  return (currentTick - startTick_) / 1000.0;
};

void TimeService::Sleep(int msecs) { sleep_ms(msecs); };
