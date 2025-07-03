/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _BITMAPGFX_H
#define _BITMAPGFX_H

#include "chargfx.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void bitmapgfx_draw_bitmap(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                           const uint8_t *bitmap_data, uint16_t fg_color,
                           uint16_t bg_color);

#ifdef __cplusplus
}
#endif

#endif // _BITMAPGFX_H