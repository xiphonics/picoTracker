/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _TIMER_H_
#define _TIMER_H_

#include "Foundation/Observable.h"
#include "Foundation/T_Factory.h"

typedef void (*timerCallback)();

class I_Timer : public Observable { // Interface for a single timer
public:
  virtual ~I_Timer();

protected:
  I_Timer();

public:
  virtual void SetPeriod(float msec) = 0;
  virtual bool Start() = 0;
  virtual void Stop() = 0;
  virtual float GetPeriod() = 0;
};

class TimerService : public T_Factory<TimerService> {
public:
  virtual I_Timer *CreateTimer() = 0; // Returns a timer
  virtual void TriggerCallback(int msec, timerCallback cb) = 0;
};

#endif
