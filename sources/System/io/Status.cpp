/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "Status.h"
#include <System/Console/Trace.h>
#include <System/Console/nanoprintf.h>
#include <System/System/System.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

uint32_t Status::dismiss_time_ = 0;
char Status::buffer[128];

void Status::AwaitDismiss() {
  while ((int32_t)(dismiss_time_ - System::GetInstance()->Millis()) > 0) {
    // busy-wait
  }
  dismiss_time_ = 0;
  Trace::Log("STATUS", "Hiding the Status @ %u",
             System::GetInstance()->Millis());
}

void Status::SetInternal(const char *fmt, va_list args) {
  auto status = Status::GetInstance();
  if (!status)
    return;

  npf_vsnprintf(buffer, sizeof(buffer), fmt, args);
  status->Print(buffer);
}

void Status::SetWithTimeout(uint32_t timeout_ms, const char *fmt, ...) {
  // do not override already running timers to allow status updates while shown
  if (dismiss_time_ == 0) {
    dismiss_time_ = System::GetInstance()->Millis() + timeout_ms;
    Trace::Log("STATUS", "Status will dismiss in %u @ %u ms", timeout_ms,
               System::GetInstance()->Millis());
  }

  va_list args;
  va_start(args, fmt);
  SetInternal(fmt, args);
  va_end(args);
}

void Status::Set(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  SetInternal(fmt, args);
  va_end(args);
}