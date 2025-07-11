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
#include <cstdint>

#include "FreeRTOS.h"
#include "main.h"
#include "semphr.h"
#include "usart.h"

#define BOOTLOADER_ADDR 0x81e0000U

typedef void (*pFunction)(void);

int32_t platform_get_rand();

void platform_reboot();

void platform_bootloader();

SysMutex *platform_mutex();

uint32_t millis(void);
uint32_t micros(void);

void pt_uart_putc(int c, void *context);
#endif
