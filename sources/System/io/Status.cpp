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

uint32_t Status::dismiss_time_ = 0;

void Status::AwaitDismiss() {
  while (dismiss_time_ &&
         (int32_t)(dismiss_time_ - System::GetInstance()->Millis()) > 0) {
    // busy-wait
  }
  // clear timer
  dismiss_time_ = 0;
}

void Status::SetTimeout(uint32_t timeout_ms) {
  dismiss_time_ = System::GetInstance()->Millis() + timeout_ms;
}

void Status::Set(const char *fmt, ...) {
  auto status = Status::GetInstance();
  if (!status)
    return;

  va_list args;
  va_start(args, fmt);
  char buffer[128];
  npf_vsnprintf(buffer, sizeof(buffer), fmt, args);
  status->Print(buffer);
  va_end(args);
}