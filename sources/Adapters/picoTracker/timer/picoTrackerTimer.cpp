/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "picoTrackerTimer.h"
#include "System/Console/Trace.h"
#include "System/Console/n_assert.h"
#include "System/System/System.h"

int64_t picoTrackerTimerCallback(int32_t interval, void *param) {
  picoTrackerTimer *timer = (picoTrackerTimer *)param;
  return timer->OnTimerTick();
};

int64_t picoTrackerTriggerCallback(int32_t interval, void *param) {
  timerCallback tc = (timerCallback)param;
  (*tc)();
  return 0;
};

picoTrackerTimer::picoTrackerTimer() {
  period_ = -1;
  timer_ = 0;
  running_ = false;
};

picoTrackerTimer::~picoTrackerTimer() {}

void picoTrackerTimer::SetPeriod(float msec) {
  period_ = int64_t(msec);
  offset_ = 0;
};

bool picoTrackerTimer::Start() {
  if (period_ > 0) {
    offset_ = period_;
    int64_t newcb = int(offset_);
    offset_ -= newcb;
    timer_ = add_alarm_in_ms(newcb, picoTrackerTimerCallback, this, false);
    lastTick_ = System::GetInstance()->GetClock();
    running_ = true;
  }
  return (timer_ != 0);
};

void picoTrackerTimer::Stop() {
  cancel_alarm(timer_);
  timer_ = 0;
  running_ = false;
};

float picoTrackerTimer::GetPeriod() { return period_; };

int64_t picoTrackerTimer::OnTimerTick() {
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

I_Timer *picoTrackerTimerService::CreateTimer() {
  static picoTrackerTimer timerInstance;
  timerInstance.Stop();
  timerInstance.SetPeriod(-1.0f);
  return &timerInstance;
};

void picoTrackerTimerService::TriggerCallback(int msec, timerCallback cb) {
  add_alarm_in_ms(msec, picoTrackerTriggerCallback, (void *)cb, false);
}
