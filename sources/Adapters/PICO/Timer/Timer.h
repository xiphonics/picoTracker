#ifndef _HW_TIMER_H_
#define _HW_TIMER_H_

#include "System/Timer/Timer.h"
#include "pico/stdlib.h"

class HWTimer : public I_Timer {
public:
  HWTimer();
  virtual ~HWTimer();
  virtual void SetPeriod(float msec);
  virtual bool Start();
  virtual void Stop();
  virtual float GetPeriod();
  int64_t OnTimerTick();

private:
  float period_;
  float offset_;     // Float offset taking into account
                     // period is an int
  alarm_id_t timer_; // NULL if not running
  long lastTick_;
  bool running_;
};

class HWTimerService : public TimerService {
public:
  virtual I_Timer *CreateTimer(); // Returns a timer
  virtual void TriggerCallback(int msec, timerCallback cb);
};

#endif
