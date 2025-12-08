/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _STATUS_H_
#define _STATUS_H_

#include "Foundation/T_Factory.h"
#include <stdarg.h>
#include <stdint.h>
class Status : public T_Factory<Status> {
public:
  virtual void Print(char *) = 0;
  static void Set(const char *fmt, ...);
  static void SetWithTimeout(uint32_t timeout_ms, const char *fmt, ...);
  ;
  static void AwaitDismiss();

private:
  static void SetInternal(const char *fmt, va_list args);
  static uint32_t dismiss_time_;
  static char buffer[128];
};

#define STATUS_MIN_TIME_INFO 1500

#endif
