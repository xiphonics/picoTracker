/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "advEventManager.h"
#include "Adapters/adv/audio/record.h"
#include "Adapters/adv/midi/advMidiService.h"
#include "Adapters/adv/system/input.h"
#include "Adapters/adv/utils/utils.h"
#include "Application/Application.h"
#include "Application/Model/Config.h"
#include "Services/Midi/MidiService.h"
#include "advGUIWindowImp.h"
#include "etl/map.h"
#include "platform.h"
#include "tim.h"
#include "timers.h"
#include "tusb.h"
#include <UIFramework/SimpleBaseClasses/EventManager.h>

#ifdef SERIAL_REPL
#include "SerialDebugUI.h"
#endif

#ifdef USB_REMOTE_UI
#include "picoRemoteUI.h"
#endif

#define USB_PROCESSING_INTERVAL_MS 10

bool advEventManager::finished_ = false;
bool advEventManager::redrawing_ = false;
uint16_t advEventManager::buttonMask_ = 0;

bool advEventManager::isRepeating_ = false;
unsigned long advEventManager::time_ = 0;
unsigned int advEventManager::keyRepeat_ = 25;
unsigned int advEventManager::keyDelay_ = 500;
unsigned int advEventManager::keyKill_ = 5;
// repeating_timer_t advEventManager::timer_ =    repeating_timer_t();
static TimerHandle_t timer;
static StaticTimer_t timerBuffer;
#ifdef RTOS_STATS
static TimerHandle_t timerStats;
static StaticTimer_t timerStatsBuffer;
#endif

#ifdef SERIAL_REPL
SerialDebugUI advEventManager::serialDebugUI_ = SerialDebugUI();
char advEventManager::inBuffer[INPUT_BUFFER_SIZE] = {0};
#endif

QueueHandle_t eventQueue;

#ifdef SERIAL_REPL
#define INPUT_BUFFER_SIZE 80
char inBuffer[INPUT_BUFFER_SIZE];
#endif

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that
 * is used by the Idle task. */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize) {
  /* If the buffers to be provided to the Idle task are declared inside this
   * function then they must be declared static - otherwise they will be
   * allocated on the stack and so not exists after this function exits. */
  static StaticTask_t xIdleTaskTCB;
  static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

  /* Pass out a pointer to the StaticTask_t structure in which the Idle
   * task's state will be stored. */
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

  /* Pass out the array that will be used as the Idle task's stack. */
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;

  /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
   * Note that, as the array is necessarily of type StackType_t,
   * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that
 * is used by the Idle task. */
void vApplicationGetPassiveIdleTaskMemory(
    StaticTask_t **ppxPassiveIdleTaskTCBBuffer,
    StackType_t **ppxPassiveIdleTaskStackBuffer,
    uint32_t *pulPassiveIdleTaskStackSize, BaseType_t xPassiveIdleTaskIndex) {
  /* If the buffers to be provided to the Idle task are declared inside this
   * function then they must be declared static - otherwise they will be
   * allocated on the stack and so not exists after this function exits. */
  static StaticTask_t xPassiveIdleTaskTCB;
  static StackType_t uxPassiveIdleTaskStack[configMINIMAL_STACK_SIZE];

  /* Pass out a pointer to the StaticTask_t structure in which the Idle
   * task's state will be stored. */
  *ppxPassiveIdleTaskTCBBuffer = &xPassiveIdleTaskTCB;

  /* Pass out the array that will be used as the Idle task's stack. */
  *ppxPassiveIdleTaskStackBuffer = uxPassiveIdleTaskStack;

  /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
   * Note that, as the array is necessarily of type StackType_t,
   * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
  *pulPassiveIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/**
 * @brief This is to provide the memory that is used by the RTOS daemon/time
 * task.
 *
 * If configUSE_STATIC_ALLOCATION is set to 1, so the application must provide
 * an implementation of vApplicationGetTimerTaskMemory() to provide the memory
 * that is used by the RTOS daemon/time task.
 */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize) {
  /* If the buffers to be provided to the Timer task are declared inside this
   * function then they must be declared static - otherwise they will be
   * allocated on the stack and so not exists after this function exits. */
  static StaticTask_t xTimerTaskTCB;
  static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

  /* Pass out a pointer to the StaticTask_t structure in which the Idle
   * task's state will be stored. */
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

  /* Pass out the array that will be used as the Timer task's stack. */
  *ppxTimerTaskStackBuffer = uxTimerTaskStack;

  /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
   * Note that, as the array is necessarily of type StackType_t,
   * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

extern "C" void vApplicationIdleHook(void) {
  static uint32_t idleCounter = 0;
  if (++idleCounter % 10000 == 0) {
    Trace::Debug("Idle task alive\n");
  }
}

#ifdef RTOS_STATS
// Declared in FreeRTOSConfig.h
uint32_t GetRuntimeCounterValue(void) { return __HAL_TIM_GET_COUNTER(&htim2); }
#define MAX_TASKS 10
static TaskStatus_t pxTaskStatusArray[MAX_TASKS];
static TaskStatus_t pxTaskStatusArrayPrev[MAX_TASKS];
static etl::map<etl::string<20>, unsigned long, MAX_TASKS> stats{};
static unsigned long prevRuntime = 0;
void timerStatsHandler(TimerHandle_t xTimer) {
  UNUSED(xTimer);

  volatile UBaseType_t uxArraySize, x;
  unsigned long ulTotalRunTime, ulStatsAsPercentage, deltaPercentage;

  //   Take a snapshot of the number of tasks in case it changes while this
  //     function is executing.
  uxArraySize = uxTaskGetNumberOfTasks();
  // Generate raw status information about each task.
  uxArraySize =
      uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalRunTime);

  // For percentage calculations.
  ulTotalRunTime /= 100UL;
  //  Avoid divide by zero errors.

  Trace::Debug("%-16s %12s %12s %12s %12s", "Name", "time", "life", "5sec",
               "stack high watermark");
  if (ulTotalRunTime > 0) {
    // For each populated position in the pxTaskStatusArray array,
    // format the raw data as human readable ASCII data.
    for (uint32_t x = 0; x < uxArraySize; x++) {
      if (stats.contains(pxTaskStatusArray[x].pcTaskName)) {
        deltaPercentage = (pxTaskStatusArray[x].ulRunTimeCounter -
                           stats[pxTaskStatusArray[x].pcTaskName]) /
                          (ulTotalRunTime - prevRuntime);
      };
      stats[pxTaskStatusArray[x].pcTaskName] =
          pxTaskStatusArray[x].ulRunTimeCounter;

      //       What percentage of the total run time has the task used?
      //         This will always be rounded down to the nearest integer.
      //         ulTotalRunTimeDiv100 has already been divided by 100.
      ulStatsAsPercentage =
          pxTaskStatusArray[x].ulRunTimeCounter / ulTotalRunTime;
      Trace::Debug("%-16s %12lu %11lu%% %11lu%% %12lu",
                   pxTaskStatusArray[x].pcTaskName,
                   pxTaskStatusArray[x].ulRunTimeCounter, ulStatsAsPercentage,
                   deltaPercentage, pxTaskStatusArray[x].usStackHighWaterMark);
    }
    prevRuntime = ulTotalRunTime;
  }
}
#endif

// timer callback at a rate of PICO_CLOCK_HZ
void timerHandler(TimerHandle_t xTimer) {
  UNUSED(xTimer);
  Event ev(CLOCK);
  xQueueSend(eventQueue, &ev, 0);
}

/*
int readFromUSBCDC(char *buf, int len) {
  int rc = PICO_ERROR_NO_DATA;
  if (tud_cdc_available()) {
    int count = (int)tud_cdc_read(buf, (uint32_t)len);
    rc = count ? count : PICO_ERROR_NO_DATA;
  }
  return rc;
}
*/

void ProcessEvent(void *) {
  uint8_t eventQueueStorage[EVENT_QUEUE_LENGTH * EVENT_QUEUE_ITEM_SIZE];
  eventQueue = xQueueCreateStatic(EVENT_QUEUE_LENGTH, EVENT_QUEUE_ITEM_SIZE,
                                  eventQueueStorage, &eventQueueBuffer);

  Event event(EventType::LAST);
  for (;;) {
    if (xQueueReceive(eventQueue, &event, portMAX_DELAY) == pdTRUE) {
      advGUIWindowImp::ProcessEvent(event);
    }
  }
}

void USBDevice(void *) {
  for (;;) {
    tud_task(); // Handle USB device events
    vTaskDelay(pdMS_TO_TICKS(USB_PROCESSING_INTERVAL_MS));
  }
}

advEventManager::advEventManager() {}

advEventManager::~advEventManager() {}

bool advEventManager::Init() {
  EventManager::Init();
  keyboardCS_ = new KeyboardControllerSource("keyboard");

#ifdef RTOS_STATS
  timerStats =
      xTimerCreateStatic("StatsTimer", 5000 / portTICK_PERIOD_MS, pdTRUE,
                         (void *)0, timerStatsHandler, &timerStatsBuffer);
  xTimerStart(timerStats, 100);
#endif

  // TODO: fix this, there is a timer service that should be used. Also all of
  // this keyRepeat logic is already implemented in the eventdispatcher
  // Application/Commands/EventDispatcher.cpp
  //  add_repeating_timer_ms(1, timerHandler, NULL, &timer_);
  timer =
      xTimerCreateStatic(/* Just a text name, not used by the RTOS kernel. */
                         "advTimer",
                         /* The timer period in ticks, must be greater than
                         0.
                          */
                         PICO_CLOCK_INTERVAL, // PICO_CLOCK_HZ timer for
                                              // timeHandler to create UI redraw
                                              // events

                         /* The timers will auto-reload themselves when they
                          * expire.
                          */
                         pdTRUE,
                         /* The ID is used to store a count of the number of
                            times the timer has expired, which is initialised
                            to 0. */
                         (void *)0,
                         /* Each timer calls the same callback when it
                         expires.
                          */
                         timerHandler, &timerBuffer);

  if (xTimerStart(timer, pdMS_TO_TICKS(100)) != pdPASS) {
    Trace::Error("Failed to start timer");
    return false;
  } else {
    Trace::Debug("Timer started with period: %d ticks", PICO_CLOCK_INTERVAL);
    return true;
  }
}

int advEventManager::MainLoop() {
  int loops = 0;
  int events = 0;
#ifdef SDIO_BENCH
  // Perform a benchmark of SD card on startup
  sd_bench();
#endif

  static StackType_t InputEventsStack[2000];
  static StaticTask_t InputEventsTCB;
  xTaskCreateStatic(ProcessInputEvent, "InEvent", 2000, NULL, 2,
                    InputEventsStack, &InputEventsTCB);

#ifdef SERIAL_REPL
  static StackType_t SerialDebugInputStack[1000];
  static StaticTask_t SerialDebugInputTCB;
  xTaskCreateStatic(ProcessSerialInputEvent, "SerialInEvent", 1000, NULL, 2,
                    SerialDebugInputStack, &SerialDebugInputTCB);
#endif

  static StackType_t ProcessEventStack[1000];
  static StaticTask_t ProcessEventTCB;
  xTaskCreateStatic(ProcessEvent, "ProcEvent", 1000, NULL, 1, ProcessEventStack,
                    &ProcessEventTCB);

  RecordHandle = xTaskCreateStatic(Record, "Record", 1000, NULL, 1, RecordStack,
                                   &RecordTCB);

  static StackType_t USBDeviceStack[512];
  static StaticTask_t USBDeviceTCB;
  xTaskCreateStatic(USBDevice, "USB Device", 512, NULL, tskIDLE_PRIORITY + 2,
                    USBDeviceStack, &USBDeviceTCB);

  vTaskStartScheduler();
  // we never get here

  // TODO: HW Shutdown
  return 0;
}

void advEventManager::PostQuitMessage() {
  // Trace:Log("EVENT", "quit");
  finished_ = true;
}

int advEventManager::GetKeyCode(const char *name) { return -1; }

void advEventManager::ProcessSerialInputEvent(void *) {
  MidiService *midiService = MidiService::GetInstance();
  for (;;) {
#ifdef SERIAL_REPL
    // Process serial debug input
    serialDebugUI_.readSerialIn(inBuffer, INPUT_BUFFER_SIZE);

    vTaskDelay(pdMS_TO_TICKS(50)); // process at approx 20Hz
#endif

    // Process UART input for remote UI if enabled
#ifdef USB_REMOTE_UI
    uint8_t uartBuffer[16];
    HAL_StatusTypeDef status =
        HAL_UART_Receive(&huart1, uartBuffer, sizeof(uartBuffer), 0);
    if (status == HAL_OK) {
      Trace::Debug("Received %d bytes from UART", sizeof(uartBuffer));
      // For now, we'll just trigger a redraw when any data is received
      // You can add more sophisticated command handling here as needed
      Event ev(REDRAW);
      xQueueSend(eventQueue, &ev, 0);
    }
#endif // USB_REMOTE_UI

    // Poll MIDI service to process any pending MIDI messages
    if (midiService) {
      advMidiService *ptMidiService = (advMidiService *)midiService;
      if (ptMidiService) {
        ptMidiService->poll();
      }
    }
  }
}

void advEventManager::ProcessInputEvent(void *) {
  for (;;) {
    uint16_t newMask, sendMask;

    if (redrawing_)
      return;
    bool gotEvent = false;

    // Get current mask
    newMask = scanKeys();

    // compute mask to send
    sendMask = (newMask ^ buttonMask_) |
               (newMask & (KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN));
    unsigned long now = millis();
    // see if we're repeating
    if (newMask == buttonMask_) {
      if ((isRepeating_) && ((now - time_) > keyRepeat_)) {
        gotEvent = (sendMask != 0);
      }
      if ((!isRepeating_) && ((now - time_) > keyDelay_)) {
        gotEvent = (sendMask != 0);
        if (gotEvent)
          isRepeating_ = true;
      }
    } else {
      if ((now - time_) > keyKill_) {
        gotEvent = (sendMask != 0);
        if (gotEvent)
          isRepeating_ = false;
      }
    }
    if (gotEvent) {
      // Get time here so delay is independant of processing speed
      time_ = millis();

      //                Trace::Debug("Pe") ;
      advGUIWindowImp::ProcessButtonChange(sendMask, newMask);
      buttonMask_ = newMask;
      //            Trace::Debug("%d: mask=%x",gTime_,sendMask) ;
      //                Trace::Debug("~Pe") ;
    }
    //    Trace::Debug("Input task running, stack free: %d\n",
    //                 uxTaskGetStackHighWaterMark(NULL));
    //    Trace::Debug("Tick count: %lu\n", xTaskGetTickCount());
    vTaskDelay(pdMS_TO_TICKS(50)); // check input at 20Hz
    //    Trace::Debug("Inputs task");
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == SD_DET_Pin) {
    if (HAL_GPIO_ReadPin(SD_DET_GPIO_Port, SD_DET_Pin) == GPIO_PIN_RESET) {
      // SD card inserted
      Event ev(SD_DET);
      xQueueSend(eventQueue, &ev, 0);
    } else {
      // We don't yet do anything for SD Card removed, could actually unlink
      // FS on removal
    }
  }
}
