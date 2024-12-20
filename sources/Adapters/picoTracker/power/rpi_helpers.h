/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Code taken from pico-extras dormant mode helper functions

#include <cstdint>
#ifndef _RPI_HELPERS_H_
#define _RPI_HELPERS_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  DORMANT_SOURCE_NONE,
  DORMANT_SOURCE_XOSC,
  DORMANT_SOURCE_ROSC,
  DORMANT_SOURCE_LPOSC, // rp2350 only
} dormant_source_t;

void ptsleep_goto_dormant_until_pin(uint8_t gpio_pin, bool edge, bool high);

void sleep_run_from_dormant_source(dormant_source_t dormant_source);

void sleep_power_up(void);
#ifdef __cplusplus
}
#endif

#endif