/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "Foundation/T_Factory.h"
#include "typedefs.h"
#include <stdint.h>
#include <stdlib.h>

struct BatteryState {
  uint8_t percentage;
  uint16_t voltage_mv;   // millivolts
  int16_t temperature_c; // celsius
  bool charging;
};

class System : public T_Factory<System> {

public:                                 // Override in implementation
  virtual unsigned long GetClock() = 0; // millisecs
  virtual void GetBatteryState(BatteryState &state) = 0;
  virtual void SetDisplayBrightness(unsigned char value) = 0;
  virtual void *Malloc(unsigned size) = 0;
  virtual void Free(void *) = 0;
  virtual void Memset(void *addr, char value, int size) = 0;
  virtual void *Memcpy(void *s1, const void *s2, int n) = 0;
  virtual void PostQuitMessage() = 0;
  virtual unsigned int GetMemoryUsage() = 0;
  virtual void PowerDown() = 0;
  virtual void SystemPutChar(int c) = 0;
  virtual void SystemBootloader() = 0;
  virtual void SystemReboot() = 0;
  virtual int32_t GetRandomNumber() = 0;
  virtual uint32_t Micros() = 0;
  virtual uint32_t Millis() = 0;
};

#define SYS_MEMSET(a, b, c)                                                    \
  {                                                                            \
    System *system = System::GetInstance();                                    \
    system->Memset(a, b, c);                                                   \
  }
#define SYS_MEMCPY(a, b, c)                                                    \
  {                                                                            \
    System *system = System::GetInstance();                                    \
    system->Memcpy(a, b, c);                                                   \
  }
#define SYS_MALLOC(size) (System::GetInstance()->Malloc(size))
#define SYS_FREE(ptr) (System::GetInstance()->Free(ptr))

#define SAFE_DELETE(ptr)                                                       \
  if (ptr) {                                                                   \
    delete ptr;                                                                \
    ptr = 0;                                                                   \
  }
#define SAFE_FREE(ptr)                                                         \
  if (ptr) {                                                                   \
    SYS_FREE(ptr);                                                             \
    ptr = 0;                                                                   \
  }

#endif
