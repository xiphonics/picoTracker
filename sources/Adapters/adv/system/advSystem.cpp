/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "advSystem.h"
#include "Adapters/adv/audio/advAudio.h"
#include "Adapters/adv/filesystem/advFileSystem.h"
#include "Adapters/adv/gui/GUIFactory.h"
#include "Adapters/adv/midi/advMidiService.h"
#include "Adapters/adv/system/advSamplePool.h"
#include "Adapters/adv/timer/advTimer.h"
#include "Application/Commands/NodeList.h"
#include "Application/Controllers/ControlRoom.h"
#include "Application/Model/Config.h"
#include "Application/Player/SyncMaster.h"
#include "BatteryGauge.h"
#include "charger.h"
#include "critical_error_message.h"
#include "input.h"
#include "platform.h"
#include "rng.h"
#include "rtc.h"
#include "tim.h"
#include "tlv320aic3204.h"
#include <assert.h>
#include <fcntl.h>
#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define ONBOOT_MINIMUM_ALLOWED_BATTERY_PERCENTAGE 3
#define DISPLAY_LOWBATT_DELAY_IN_SEC 5

EventManager *advSystem::eventManager_ = NULL;
bool advSystem::invert_ = false;
int advSystem::lastBattLevel_ = 100;
unsigned int advSystem::lastBeatCount_ = 0;

int advSystem::MainLoop() {

  eventManager_->InstallMappings();
  return eventManager_->MainLoop();
};

void advSystem::Boot() {

  // Start high resolution timer
  HAL_TIM_Base_Start(&htim2);

  // Install System
  __attribute__((
      section(".DATA_RAM"))) static char systemMemBuf[sizeof(advSystem)];
  System::Install(new (systemMemBuf) advSystem());

  // Install GUI Factory
  __attribute__((
      section(".DATA_RAM"))) static char guiMemBuf[sizeof(GUIFactory)];
  I_GUIWindowFactory::Install(new (guiMemBuf) GUIFactory());

  // Install Timers
  __attribute__((
      section(".DATA_RAM"))) static char timerMemBuf[sizeof(advTimerService)];
  TimerService::GetInstance()->Install(new (timerMemBuf) advTimerService());

  // Install FileSystem
  __attribute__((
      section(".DATA_RAM"))) static char fsMemBuf[sizeof(advFileSystem)];
  FileSystem::Install(new (fsMemBuf) advFileSystem());

  // First check for SDCard
  auto fs = FileSystem::GetInstance();
  if (!fs->chdir("/")) {
    Trace::Log("PICOTRACKERSYSTEM", "SDCARD MISSING!!\n");
    critical_error_message("SDCARD MISSING", 0x01,
                           DEFAULT_ERROR_MESSAGE_DELAY_SEC, false);
  }

  // Install MIDI
  // **NOTE**: MIDI install MUST happen before Audio install because it triggers
  // reading config file and config file needs to have MidiService already
  // installed in order to apply midi settings read from the config file
  __attribute__((
      section(".DATA_RAM"))) static char midiMemBuf[sizeof(advMidiService)];
  MidiService::Install(new (midiMemBuf) advMidiService());

  // Install Sound
  AudioSettings hint;
  hint.bufferSize_ = 1024;
  hint.preBufferCount_ = 8;
  __attribute__((
      section(".DATA_RAM"))) static char audioMemBuf[sizeof(advAudio)];
  Audio::Install(new (audioMemBuf) advAudio(hint));

  // Install SamplePool
  static char samplePoolMemBuf[sizeof(advSamplePool)];
  SamplePool::Install(new (samplePoolMemBuf) advSamplePool());

  // Handle wake up
  if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB)) {
    // Determine actual source
    if (PWR->WKUPFR & PWR_WKUPFR_WKUPF2) {
      Trace::Log("POWERON", "Woke up from power button");
      HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
    } else if (__HAL_RTC_WAKEUPTIMER_GET_FLAG(&hrtc, RTC_FLAG_WUTF)) {
      Trace::Log("POWERON", "Woke up from RTC - back to sleep");
      powerOff();
    } else {
      Trace::Log("POWERON", "Woke up from unknown source");
    }
  } else {
    Trace::Log("POWERON", "Woke up from deep sleep");
  }

  // Enable display PWM
  HAL_TIM_PWM_Start(&htim13, TIM_CHANNEL_1);

  // Configure the battery fuel gauge - will only update if ITPOR bit is set
  configureBatteryGauge();

  // check for low batt
  BatteryState batteryState;
  System::GetInstance()->GetBatteryState(batteryState);
  if (batteryState.percentage < ONBOOT_MINIMUM_ALLOWED_BATTERY_PERCENTAGE &&
      !batteryState.charging) {
    // show low battery message on screen
    Trace::Log("PICOTRACKERSYSTEM", "Low Batt: %d%%\n",
               batteryState.percentage);
    critical_error_message("!! LOW BATTERY !!", 0x01,
                           DISPLAY_LOWBATT_DELAY_IN_SEC, false);
  }

  eventManager_ = I_GUIWindowFactory::GetInstance()->GetEventManager();
  eventManager_->Init();
};

void advSystem::Shutdown() { delete Audio::GetInstance(); };

static int secbase;

unsigned long advSystem::GetClock() {
  struct timeval tp;

  gettimeofday(&tp, NULL);
  if (!secbase) {
    secbase = tp.tv_sec;
    return long(tp.tv_usec / 1000.0);
  }
  return long((tp.tv_sec - secbase) * 1000 + tp.tv_usec / 1000.0);
}

void advSystem::GetBatteryState(BatteryState &state) {
  auto soc = getBatterySOC();
  state.voltage_mv = getBatteryVoltage();
  state.temperature_c = getBatteryTemperature();
  ChargingStatus status = getChargingStatus();
  if (status == PRE_CHARGE || status == FAST_CHARGE) {
    state.charging = true;
  } else {
    state.charging = false;
  }
  if (soc < 0) {
    state.error = true;
  } else {
    state.percentage = soc;
    state.error = false;
  }
}

void advSystem::SetDisplayBrightness(unsigned char value) {
  platform_brightness(value);
}

void advSystem::Sleep(int millisec) {
  //	if (millisec>0)
  //		assert(0) ;
}

void *advSystem::Malloc(unsigned size) {
  void *ptr = malloc(size);
  return ptr;
}

void advSystem::Free(void *ptr) { free(ptr); }

void advSystem::Memset(void *addr, char val, int size) {
  memset(addr, val, size);
};

void *advSystem::Memcpy(void *s1, const void *s2, int n) {
  return memcpy(s1, s2, n);
}

void advSystem::PostQuitMessage() { eventManager_->PostQuitMessage(); }

unsigned int advSystem::GetMemoryUsage() {
  struct mallinfo m = mallinfo();
  return m.uordblks;
}

void advSystem::PowerDown() { powerOff(); }

void advSystem::SystemPutChar(int c) {
  HAL_UART_Transmit(&DEBUG_UART, (uint8_t *)&c, 1, 0x000F);
}

uint32_t advSystem::GetRandomNumber() {
  uint32_t random32;
  if (HAL_RNG_GenerateRandomNumber(&hrng, &random32) == HAL_OK) {
    return (int32_t)random32;
  } else {
    Trace::Error("Error generating random number");
    return 0;
  }
}

void advSystem::SystemBootloader() { platform_bootloader(); }

void advSystem::SystemReboot() { platform_reboot(); }

uint32_t advSystem::Micros() { return micros(); }

uint32_t advSystem::Millis() { return millis(); }
