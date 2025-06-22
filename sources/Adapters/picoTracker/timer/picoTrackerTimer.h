/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _PICOTRACKERTIMER_H_
#define _PICOTRACKERTIMER_H_

#include "System/Timer/Timer.h"
#include "pico/stdlib.h"

class picoTrackerTimer : public I_Timer {
public:
  picoTrackerTimer();
  virtual ~picoTrackerTimer();
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

class picoTrackerTimerService : public TimerService {
public:
  virtual I_Timer *CreateTimer(); // Returns a timer
  virtual void TriggerCallback(int msec, timerCallback cb);
};

#endif
