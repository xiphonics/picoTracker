/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "Status.h"
#include <System/Console/nanoprintf.h>
#include <stdarg.h>
#include <stdio.h>

void Status::Set(const char *fmt, ...) {

  Status *status = Status::GetInstance();
  if (!status)
    return;

  char buffer[128];
  va_list args;
  va_start(args, fmt);

  npf_vsnprintf(buffer, sizeof(buffer), fmt, args);
  status->Print(buffer);

  va_end(args);
}

void Status::SetMultiLine(const char *fmt, ...) {
  Status *status = Status::GetInstance();
  if (!status)
    return;

  char buffer[128];
  va_list args;
  va_start(args, fmt);

  npf_vsnprintf(buffer, sizeof(buffer), fmt, args);
  status->PrintMultiLine(buffer);

  va_end(args);
}
