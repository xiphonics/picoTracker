/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "Adapters/picoTracker/platform/platform.h"
#include "Adapters/picoTracker/system/picoTrackerSystem.h"
#include "Application/Application.h"
#include "bsp/board.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "tusb.h"
#include <System/Console/Trace.h>

#include "../system/input.h"

// this prevents a really annoying linker warning due to newlib and >gcc13
// ref:
// https://github.com/raspberrypi/pico-sdk/issues/1768#issuecomment-2649260970
#ifdef __cplusplus
extern "C" {
int _getentropy(void *buffer, size_t length) { return -1; }
}
#endif
// ================

int main(int argc, char *argv[]) {

  // Initialise microcontroller specific hardware
  board_init();

  // Initialise TinyUSB
  tusb_init();

  // Do remaining pT init, this needs to be done *after* above hardware and
  // tinyusb subsystem init
  platform_init();

  // Check for ENTER key hold on boot to force load untitled project
  {
    int enter_held_counter = 0;
    const int check_interval_ms = 10;
    const int num_checks = 20;      // check for 200ms
    const int required_checks = 15; // require 150ms of hold time

    for (int i = 0; i < num_checks; i++) {
      uint16_t keys = scanKeys();
      if (keys & (1 << 6)) { // Check for INPUT_ENTER (bit 6)
        enter_held_counter++;
      } else {
        enter_held_counter = 0;
      }

      if (enter_held_counter >= required_checks) {
        forceLoadUntitledProject = true;
        break;
      }
      sleep_ms(check_interval_ms);
    }
  }

  // Make sure we get ETL logs
  Trace::RegisterEtlErrorHandler();

  picoTrackerSystem::Boot(argc, argv);

  GUICreateWindowParams params;
  params.title = "picoTracker";

  Application::GetInstance()->Init(params);

  picoTrackerSystem::MainLoop();
  // WE NEVER GET HERE

  picoTrackerSystem::Shutdown();
}
