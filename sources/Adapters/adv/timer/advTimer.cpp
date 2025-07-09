#include "advTimer.h"
#include "System/Console/Trace.h"
#include "System/Console/n_assert.h"
#include "System/System/System.h"

int64_t advTimerCallback(int32_t interval, void *param) {
  advTimer *timer = (advTimer *)param;
  return timer->OnTimerTick();
};

int64_t advTriggerCallback(int32_t interval, void *param) {
  timerCallback tc = (timerCallback)param;
  (*tc)();
  return 0;
};

advTimer::advTimer() {
  period_ = -1;
  timer_ = 0;
  running_ = false;
};

advTimer::~advTimer() {}

void advTimer::SetPeriod(float msec) {
  period_ = int64_t(msec);
  offset_ = 0;
};

bool advTimer::Start() {
  if (period_ > 0) {
    offset_ = period_;
    int64_t newcb = int(offset_);
    offset_ -= newcb;
    // TODO(stm): reenable this! what to do with callback?
    //    timer_ = add_alarm_in_ms(newcb, advTimerCallback, this,
    //    false);
    //    timer_ =
    //        xTimerCreate(/* Just a text name, not used by the RTOS kernel. */
    //                     "advTimer",
    /* The timer period in ticks, must be greater than 0. */
    //                     newcb / portTICK_PERIOD_MS,
    /* The timers will auto-reload themselves when they expire.
     */
    //                     pdTRUE,
    /* The ID is used to store a count of the number of times
       the timer has expired, which is initialised to 0. */
    //                     (void *)0,
    /* Each timer calls the same callback when it expires. */
    //                     advTimerCallback);
    //    auto result = xTimerStart(timer_, 100);
    //    Trace::Log("Timer started: %b", result);
    lastTick_ = System::GetInstance()->GetClock();
    running_ = true;
  }
  return (timer_ != 0);
};

void advTimer::Stop() {
  // TODO(stm): reenable
  //  cancel_alarm(timer_);
  //  timer_ = 0;
  //  auto result = xTimerStop(timer_, 100);
  //  Trace::Log("Timer stopped: %b", result);
  running_ = false;
};

float advTimer::GetPeriod() { return period_; };

int64_t advTimer::OnTimerTick() {
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

I_Timer *advTimerService::CreateTimer() { return new advTimer(); };

void advTimerService::TriggerCallback(int msec, timerCallback cb) {
  // TODO(stm): reenable this!
  //  add_alarm_in_ms(msec, advTriggerCallback, (void *)cb, false);
}
