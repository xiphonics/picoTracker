/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#ifndef _CRITICALERRORMESSAGE_H_
#define _CRITICALERRORMESSAGE_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_ERROR_MESSAGE_DELAY_SEC 5

void critical_error_message(const char *message, int guruId, int shutdownDelay,
                            bool showGuru);

#ifdef __cplusplus
}
#endif

#endif
