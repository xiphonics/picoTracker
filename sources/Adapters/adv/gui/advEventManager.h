/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#ifndef _ADVEVENTMANAGER_
#define _ADVEVENTMANAGER_

#include "Foundation/T_Singleton.h"
#ifdef SERIAL_REPL
#include "SerialDebugUI.h"
#endif
#include "Services/Controllers/KeyboardControllerSource.h"
#include "UIFramework/SimpleBaseClasses/EventManager.h"
#include "eventQueue.h"
#include "queue.h"
#include <string>

#ifdef __cplusplus
extern "C" {
#include "FreeRTOS.h"
#include "task.h"

// void vApplicationMallocFailedHook(void);
// void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName);
// void vApplicationIdleHook(void);
// void vApplicationTickHook(void);
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize);
void vApplicationGetPassiveIdleTaskMemory(
    StaticTask_t **ppxPassiveIdleTaskTCBBuffer,
    StackType_t **ppxPassiveIdleTaskStackBuffer,
    uint32_t *pulPassiveIdleTaskStackSize, BaseType_t xPassiveIdleTaskIndex);
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize);
}
#endif

class advEventManager : public T_Singleton<advEventManager>,
                        public EventManager {
public:
  advEventManager();
  ~advEventManager();
  virtual bool Init();
  virtual int MainLoop();
  virtual void PostQuitMessage();
  virtual int GetKeyCode(const char *name);

protected:
  static void ProcessInputEvent(void *);
  static void ProcessSerialInputEvent(void *);

private:
  static bool finished_;
  static bool redrawing_;
  static uint16_t buttonMask_;
  static unsigned int keyRepeat_;
  static unsigned int keyDelay_;
  static unsigned int keyKill_;
  static bool isRepeating_;
  static unsigned long time_;

#ifdef SERIAL_REPL
  static SerialDebugUI serialDebugUI_;
  static const int INPUT_BUFFER_SIZE = 80;
  static char inBuffer[INPUT_BUFFER_SIZE];
#endif
  KeyboardControllerSource *keyboardCS_;
};
#endif
