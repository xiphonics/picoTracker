#include "Timer.h"
#include "System/Console/Trace.h"
#include "System/Console/n_assert.h"
#include "System/System/System.h"

int64_t HWTimerCallback(int32_t interval, void *param) {
  HWTimer *timer = (HWTimer *)param;
  return timer->OnTimerTick();
};

int64_t HWTriggerCallback(int32_t interval, void *param) {
  timerCallback tc = (timerCallback)param;
  (*tc)();
  return 0;
};

HWTimer::HWTimer() {
  period_ = -1;
  timer_ = 0;
  running_ = false;
};

HWTimer::~HWTimer() {}

void HWTimer::SetPeriod(float msec) {
  period_ = int64_t(msec);
  offset_ = 0;
};

bool HWTimer::Start() {
  if (period_ > 0) {
    offset_ = period_;
    int64_t newcb = int(offset_);
    offset_ -= newcb;
    timer_ = add_alarm_in_ms(newcb, HWTimerCallback, this, false);
    lastTick_ = System::GetInstance()->GetClock();
    running_ = true;
  }
  return (timer_ != 0);
};

void HWTimer::Stop() {
  cancel_alarm(timer_);
  timer_ = 0;
  running_ = false;
};

float HWTimer::GetPeriod() { return period_; };

int64_t HWTimer::OnTimerTick() {
  int64_t newcb = 0;
  if (running_) {
    SetChanged();
    NotifyObservers();
    offset_ += period_;
    newcb = int64_t(offset_);
    offset_ -= newcb;
    NAssert(newcb > 0);
  }
  return newcb;
};

I_Timer *HWTimerService::CreateTimer() { return new HWTimer(); };

void HWTimerService::TriggerCallback(int msec, timerCallback cb) {
  add_alarm_in_ms(msec, HWTriggerCallback, (void *)cb, false);
}
