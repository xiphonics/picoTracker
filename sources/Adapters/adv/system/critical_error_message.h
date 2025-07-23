/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#ifndef _CRITICALERRORMESSAGE_H_
#define _CRITICALERRORMESSAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

void critical_error_message(const char *message, int guruId);

#ifdef __cplusplus
}
#endif

#endif
