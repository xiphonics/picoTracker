#ifndef _PLATFORM_PICO_H_
#define _PLATFORM_PICO_H_

#include "System/Process/SysMutex.h"
#include "gpio.h"
#include "pico/stdlib.h"

void platform_init();

int32_t platform_get_rand();

void platform_reboot();

void platform_bootloader();

SysMutex *platform_mutex();

void platform_brightness(uint8_t value);

void pt_uart_putc(int c, void *context);

#endif
