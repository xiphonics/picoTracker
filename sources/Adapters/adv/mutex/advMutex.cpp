/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "advMutex.h"
#include "System/Console/Trace.h"

static StaticSemaphore_t storage;

advMutex::advMutex() : mutex_(xSemaphoreCreateBinaryStatic(&storage)) {
  // Semaphore is created in an empty state
  xSemaphoreGive(mutex_);
}

bool advMutex::Lock() {
  if (xSemaphoreTake(mutex_, portMAX_DELAY)) {
    return true;
  }
  return false;
}

void advMutex::Unlock() {
  if (!xSemaphoreGive(mutex_)) {
    Trace::Error("Semaphore release error");
  }
}
