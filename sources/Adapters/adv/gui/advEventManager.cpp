#include "advEventManager.h"
#include "Adapters/adv/system/input.h"
#include "Adapters/adv/utils/utils.h"
#include "Application/Application.h"
#include "Application/Model/Config.h"
#include "advGUIWindowImp.h"
// #include "usb_utils.h"
#include "etl/map.h"
#include "platform.h"
#include "tim.h"
#include "timers.h"

#ifdef SERIAL_REPL
#include "SerialDebugUI.h"
#endif

#ifdef USB_REMOTE_UI
#include "picoRemoteUI.h"
#endif

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

advEventQueue *queue;

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

  Trace::Debug("Name\t\ttime\t\tlife\t\t5sec\t\tstack high watermark");
  if (ulTotalRunTime > 0) {
    // For each populated position in the pxTaskStatusArray array,
    // format the raw data as human readable ASCII data.
    for (x = 0; x < uxArraySize; x++) {
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
      Trace::Debug("%s\t\t%lu\t\t%lu%%\t\t%lu%%\t\t%lu",
                   pxTaskStatusArray[x].pcTaskName,
                   pxTaskStatusArray[x].ulRunTimeCounter, ulStatsAsPercentage,
                   deltaPercentage, pxTaskStatusArray[x].usStackHighWaterMark);
    }
    prevRuntime = ulTotalRunTime;
  }
}
#endif

// timer callback at a rate of 50Hz
void timerHandler(TimerHandle_t xTimer) {
  UNUSED(xTimer);
  queue = advEventQueue::GetInstance();

  // send a clock (PICO_CLOCK)
  queue->push(advEvent(PICO_CLOCK));
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
  auto queue = advEventQueue::GetInstance();
  for (;;) {
    if (!queue->empty()) {
      advEvent event(advEventType::LAST);
      queue->pop_into(event);
      //      redrawing_ = true;
      advGUIWindowImp::ProcessEvent(event);
      //      redrawing_ = false;
    }
    //    Trace::Debug("Event task running, stack free: %d\n",
    //                 uxTaskGetStackHighWaterMark(NULL));
    // TODO: the event queue should be a FreeRTOS queue and this should halt
    // waiting for an event
    vTaskDelay(50 / portTICK_PERIOD_MS);
    //    Trace::Debug("Process event");
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

  // 50Hz timer for timeHanlder to create UI redraw events
  int ticks = pdMS_TO_TICKS(20);
  timer =
      xTimerCreateStatic(/* Just a text name, not used by the RTOS kernel. */
                         "advTimer",
                         /* The timer period in ticks, must be greater than
                         0.
                          */
                         ticks, // 20ms = 50Hz
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
    Trace::Debug("Timer started with period: %d ticks", ticks);
    return true;
  }
}

int advEventManager::MainLoop() {
  queue = advEventQueue::GetInstance();
  int loops = 0;
  int events = 0;
#ifdef SDIO_BENCH
  // Perform a benchmark of SD card on startup
  sd_bench();
#endif

  static StackType_t InputEventsStack[1000];
  static StaticTask_t InputEventsTCB;
  xTaskCreateStatic(ProcessInputEvent, "InEvent", 1000, NULL, 2,
                    InputEventsStack, &InputEventsTCB);

#ifdef SERIAL_REPL
  static StackType_t SerialDebugInputStack[1000];
  static StaticTask_t SerialDebugInputTCB;
  xTaskCreateStatic(ProcessSerialDebugInputEvent, "SerialDebugInEvent", 1000,
                    NULL, 2, SerialDebugInputStack, &SerialDebugInputTCB);
#endif

  static StackType_t ProcessEventStack[1000];
  static StaticTask_t ProcessEventTCB;
  xTaskCreateStatic(ProcessEvent, "ProcEvent", 1000, NULL, 1, ProcessEventStack,
                    &ProcessEventTCB);
  vTaskStartScheduler();
  // we never get here

  while (!finished_) {
    loops++;

    // process usb interrupts, should this be done somewhere else??
    //    handleUSBInterrupts();

    //    ProcessInputEvent();
    if (!queue->empty()) {
      advEvent event(advEventType::LAST);
      queue->pop_into(event);
      events++;
      redrawing_ = true;
      advGUIWindowImp::ProcessEvent(event);
      redrawing_ = false;
    }
#ifdef PICOSTATS
    if (loops == 100000) {
      Trace::Debug("Usage %.1f% CPU\n", ((float)events / loops) * 100);
      events = 0;
      loops = 0;
      //      measure_freqs();
      measure_free_mem();
    }
#endif
  }
  // TODO: HW Shutdown
  return 0;
}

void advEventManager::PostQuitMessage() {
  // Trace:Log("EVENT", "quit");
  finished_ = true;
}

int advEventManager::GetKeyCode(const char *name) { return -1; }

#ifdef SERIAL_REPL
void advEventManager::ProcessSerialDebugInputEvent(void *) {
  for (;;) {
    // Process serial debug input
    serialDebugUI_.readSerialIn(inBuffer, INPUT_BUFFER_SIZE);

    vTaskDelay(pdMS_TO_TICKS(50)); // check input at 20Hz
  }
}
#endif

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

// Process UART input for remote UI if enabled
#ifdef USB_REMOTE_UI
    uint8_t uartBuffer[16];
    HAL_StatusTypeDef status =
        HAL_UART_Receive(&huart1, uartBuffer, sizeof(uartBuffer), 0);
    if (status == HAL_OK) {
      Trace::Debug("Received %d bytes from UART", sizeof(uartBuffer));
      // For now, we'll just trigger a redraw when any data is received
      // You can add more sophisticated command handling here as needed
      queue = advEventQueue::GetInstance();
      queue->push(advEvent(PICO_REDRAW));
    }
#endif // USB_REMOTE_UI
    //    Trace::Debug("Input task running, stack free: %d\n",
    //                 uxTaskGetStackHighWaterMark(NULL));
    //    Trace::Debug("Tick count: %lu\n", xTaskGetTickCount());
    vTaskDelay(pdMS_TO_TICKS(50)); // check input at 20Hz
    //    Trace::Debug("Inputs task");
  }
}
