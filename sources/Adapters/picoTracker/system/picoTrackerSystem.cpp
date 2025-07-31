/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "picoTrackerSystem.h"
#include "Adapters/picoTracker/audio/picoTrackerAudio.h"
#include "Adapters/picoTracker/filesystem/picoTrackerFileSystem.h"
#include "Adapters/picoTracker/gui/GUIFactory.h"
#include "Adapters/picoTracker/midi/picoTrackerMidiService.h"
#include "Adapters/picoTracker/system/picoTrackerSamplePool.h"
#include "Adapters/picoTracker/timer/picoTrackerTimer.h"
#include "Application/Commands/NodeList.h"
#include "Application/Controllers/ControlRoom.h"
#include "Application/Model/Config.h"
#include "Application/Player/SyncMaster.h"
#include "hardware/gpio.h"
#include "input.h"
#include <assert.h>
#include <fcntl.h>
#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "Adapters/picoTracker/platform/platform.h"
#include "critical_error_message.h"
#include "hardware/adc.h"
#include "pico/stdlib.h"

EventManager *picoTrackerSystem::eventManager_ = NULL;
bool picoTrackerSystem::invert_ = false;
unsigned int picoTrackerSystem::lastBeatCount_ = 0;

int picoTrackerSystem::MainLoop() {

  eventManager_->InstallMappings();
  return eventManager_->MainLoop();
};

void picoTrackerSystem::Boot(int argc, char **argv) {

  // Install System
  alignas(
      picoTrackerSystem) static char systemMemBuf[sizeof(picoTrackerSystem)];
  System::Install(new (systemMemBuf) picoTrackerSystem());

  // Install GUI Factory
  alignas(GUIFactory) static char guiMemBuf[sizeof(GUIFactory)];
  I_GUIWindowFactory::Install(new (guiMemBuf) GUIFactory());

  // Install Timers
  alignas(picoTrackerTimerService) static char
      timerMemBuf[sizeof(picoTrackerTimerService)];
  TimerService::GetInstance()->Install(new (timerMemBuf)
                                           picoTrackerTimerService());

  // Install FileSystem
  alignas(picoTrackerFileSystem) static char
      fsMemBuf[sizeof(picoTrackerFileSystem)];
  FileSystem::Install(new (fsMemBuf) picoTrackerFileSystem());

  // First check for SDCard
  auto fs = FileSystem::GetInstance();
  if (!fs->chdir("/")) {
    Trace::Log("PICOTRACKERSYSTEM", "SDCARD MISSING!!");
    critical_error_message("SDCARD MISSING", 0x01);
  }

  // Install MIDI
  // **NOTE**: MIDI install MUST happen before Audio install because it triggers
  // reading config file and config file needs to have MidiService already
  // installed in order to apply midi settings read from the config file
  alignas(picoTrackerMidiService) static char
      midiMemBuf[sizeof(picoTrackerMidiService)];
  MidiService::Install(new (midiMemBuf) picoTrackerMidiService());

  // Install Sound
  AudioSettings hint;
  hint.bufferSize_ = 1024;
  hint.preBufferCount_ = 8;
  alignas(picoTrackerAudio) static char audioMemBuf[sizeof(picoTrackerAudio)];
  Audio::Install(new (audioMemBuf) picoTrackerAudio(hint));

  // Install SamplePool
  alignas(picoTrackerSamplePool) static char
      samplePoolMemBuf[sizeof(picoTrackerSamplePool)];
  SamplePool::Install(new (samplePoolMemBuf) picoTrackerSamplePool());

  eventManager_ = I_GUIWindowFactory::GetInstance()->GetEventManager();
  eventManager_->Init();

#if PICO_RP2040
  // init GPIO for use as ADC: hi-Z, no pullups, etc
  adc_gpio_init(BATT_VOLTAGE_IN);

  adc_init();

  // select analog MUX, GPIO 26=0, 27=1, 28=1, 29=3
  adc_select_input(3);

  Trace::Log("PICOTRACKERSYSTEM", "ADC INIT DONE");
#endif

  // Initialize display brightness from config
  UpdateBrightnessFromConfig();
};

void picoTrackerSystem::Shutdown() { delete Audio::GetInstance(); };

static int secbase;

unsigned long picoTrackerSystem::GetClock() {
  struct timeval tp;

  gettimeofday(&tp, NULL);
  if (!secbase) {
    secbase = tp.tv_sec;
    return long(tp.tv_usec / 1000.0);
  }
  return long((tp.tv_sec - secbase) * 1000 + tp.tv_usec / 1000.0);
}

void picoTrackerSystem::GetBatteryState(BatteryState &state) {
  u_int16_t adc_reading = adc_read(); // raw voltage from ADC

  int adc_voltage = adc_reading * 0.8; // 0.8mV per unit of ADC
  // *2 because picoTracker use voltage divider for voltage on ADC pin
  state.voltage_mv = adc_voltage * 2;

  // we just do a very basic percentage estimation based on several voltage
  // thresholds
  if (state.voltage_mv < 3400) {
    state.percentage = 0;
  } else if (state.voltage_mv < 3500) {
    state.percentage = 30;
  } else if (state.voltage_mv < 3700) {
    state.percentage = 60;
  } else if (state.voltage_mv < 3900) {
    state.percentage = 90;
  } else {
    state.percentage = 100;
  }
  state.charging = state.voltage_mv > 4000 ? true : false;
}

void picoTrackerSystem::SetDisplayBrightness(unsigned char value) {
  platform_brightness(value);
}

void picoTrackerSystem::UpdateBrightnessFromConfig() {
  // Get the brightness value from config and apply it
  Config *config = Config::GetInstance();
  if (config) {
    Variable *v = config->FindVariable(FourCC::VarBacklightLevel);
    if (v) {
      unsigned char brightness = (unsigned char)v->GetInt();
      platform_brightness(brightness);
      Trace::Log("PICOTRACKERSYSTEM", "Set display brightness to %d",
                 brightness);
    }
  }
}

void picoTrackerSystem::Sleep(int millisec) {
  //	if (millisec>0)
  //		assert(0) ;
}

void *picoTrackerSystem::Malloc(unsigned size) {
  void *ptr = malloc(size);
  return ptr;
}

void picoTrackerSystem::Free(void *ptr) { free(ptr); }

void picoTrackerSystem::Memset(void *addr, char val, int size) {
  memset(addr, val, size);
};

void *picoTrackerSystem::Memcpy(void *s1, const void *s2, int n) {
  return memcpy(s1, s2, n);
}

void picoTrackerSystem::PostQuitMessage() { eventManager_->PostQuitMessage(); }

unsigned int picoTrackerSystem::GetMemoryUsage() {
  struct mallinfo m = mallinfo();
  return m.uordblks;
}

void picoTrackerSystem::SystemPutChar(int c) { putchar(c); }

int32_t picoTrackerSystem::GetRandomNumber() { return platform_get_rand(); }

void picoTrackerSystem::SystemBootloader() { platform_bootloader(); }

void picoTrackerSystem::SystemReboot() { platform_reboot(); }

uint32_t picoTrackerSystem::Micros() { return micros(); }

uint32_t picoTrackerSystem::Micros() { return millis(); }
