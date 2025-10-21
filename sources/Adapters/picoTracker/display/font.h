/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

typedef uint16_t font_t[96][10];

#include "font_hourglass.h"
#include "font_bold.h"
#include "font_wide.h"

static const font_t *fonts[] = {
    &FONT_HOURGLASS_BITMAP,
    &FONT_YOU_SQUARED_BITMAP,
    &FONT_WIDE_BITMAP,
};  