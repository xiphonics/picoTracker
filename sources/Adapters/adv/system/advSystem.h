/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#ifndef _ADVSYSTEM_H_
#define _ADVSYSTEM_H_

#include <map>

#include "System/System/System.h"
#include "UIFramework/SimpleBaseClasses/EventManager.h"

#define USEREVENT_TIMER 0
#define USEREVENT_EXPOSE 1

class advSystem : public System {
public:
  static void Boot();
  static void Shutdown();
  static int MainLoop();

public: // System implementation
  virtual unsigned long GetClock();
  virtual void GetBatteryState(BatteryState &state);
  virtual void SetDisplayBrightness(unsigned char value);
  virtual void Sleep(int millisec);
  virtual void *Malloc(unsigned size);
  virtual void Free(void *);
  virtual void Memset(void *addr, char val, int size);
  virtual void *Memcpy(void *s1, const void *s2, int n);
  virtual void PostQuitMessage();
  virtual unsigned int GetMemoryUsage();
  virtual void PowerDown();
  virtual void SystemBootloader();
  virtual void SystemReboot();
  virtual void SystemPutChar(int c);
  virtual int32_t GetRandomNumber();
  virtual uint32_t Micros();

private:
  static bool invert_;
  static int lastBattLevel_;
  static unsigned int lastBeatCount_;
  static EventManager *eventManager_;
  std::map<void *, unsigned> mmap_;

  void setCharging(void);
};
#endif
