/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _PLATFORM_PICO_H_
#define _PLATFORM_PICO_H_

#include "System/Process/SysMutex.h"
#include "gpio.h"
#include "pico/stdlib.h"

void platform_init();

void platform_reboot();

void platform_bootloader();

SysMutex *platform_mutex();

uint32_t millis(void);
uint32_t micros(void);

void platform_brightness(uint8_t value);

#endif
