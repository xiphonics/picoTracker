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
#include "Application/Model/Config.h"
#include "Application/Player/SyncMaster.h"
#include "hardware/gpio.h"
#include "input.h"
#include "pico/rand.h"
#include <assert.h>
#include <fcntl.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "Adapters/picoTracker/gui/InputTester.h"
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

static bool checkForSDCard() {
  drawInputTester();

  alignas(picoTrackerFileSystem) static char
      fsMemBuf[sizeof(picoTrackerFileSystem)];
  FileSystem::Install(new (fsMemBuf) picoTrackerFileSystem());

  auto fs = FileSystem::GetInstance();
  return fs->chdir("/");
}

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

  if (!fs->chdir("/") || scanKeys()) {
    Trace::Log("PICOTRACKERSYSTEM", "SDCARD MISSING!!");
    critical_error_message("SDCARD MISSING", 0x01, checkForSDCard);
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
  uint32_t adc_reading = adc_read(); // raw voltage from ADC
  // 0.8mV per unit of ADC
  // * 2 because picoTracker use voltage divider for voltage on ADC pin
  // mV =^= adc_reading * 1.6
  state.voltage_mv = (adc_reading * 8) / 5; // equals adc_reading * 1.6;

  // clamp the ends of the valid voltage range
  if (state.voltage_mv < 3325) {
    state.percentage = 0;
  } else if (state.voltage_mv > 3900) {
    state.percentage = 100;
  } else {
    // the function f(x) = 100 - (x - 3,900)^2 / 3,250 closely maps the original
    // measurements. It can be optimized for the rp2040 in integer math as
    //      100 - (100 * x - 390,000) ^ 2 / 33,000,000
    // with x / 33,000,000 being approximated by x >> 25 (2^25 = 33,554,432)
    // --> (100 - (100 * x - 390,000) ^ 2) >> 25
    uint32_t q = 100 * state.voltage_mv - 390000; // 100 * x - 390,000
    q *= q;                                       // q ^ 2
    q >>= 25;                                     // q / 33,000,000
    state.percentage = 100 - q;                   // 100 - q
  }

  state.charging = state.voltage_mv > 4000;
}

void picoTrackerSystem::SetDisplayBrightness(unsigned char value) {
  platform_brightness(value);
}

void picoTrackerSystem::Sleep(int millisec) {
  //	if (millisec>0)
  //		assert(0) ;
}

void picoTrackerSystem::PostQuitMessage() { eventManager_->PostQuitMessage(); }

unsigned int picoTrackerSystem::GetMemoryUsage() { return 0; }

void picoTrackerSystem::SystemPutChar(int c) { putchar(c); }

uint32_t picoTrackerSystem::GetRandomNumber() { return get_rand_32(); }

void picoTrackerSystem::SystemBootloader() { platform_bootloader(); }

void picoTrackerSystem::SystemReboot() { platform_reboot(); }

uint32_t picoTrackerSystem::Micros() { return micros(); }

uint32_t picoTrackerSystem::Millis() { return millis(); }
