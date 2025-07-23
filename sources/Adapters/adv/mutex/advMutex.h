/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#ifndef _ADV_MUTEX_H_
#define _ADV_MUTEX_H_

#include "FreeRTOS.h"
#include "System/Process/SysMutex.h"
#include "semphr.h"

class advMutex : public SysMutex {
public:
  advMutex();
  virtual ~advMutex(){};
  virtual bool Lock() override;
  virtual void Unlock() override;

private:
  SemaphoreHandle_t mutex_;
};

#endif // _ADV_MUTEX_H_
