/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2025 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#ifndef _ADV_CHARGER_H_
#define _ADV_CHARGER_H_

#ifdef __cplusplus
extern "C" {
#endif

void set_charging(void);
void power_off();

typedef enum {
  NOT_CHARGING,
  PRE_CHARGE,
  FAST_CHARGE,
  CHARGE_DONE
} ChargingStatus;

ChargingStatus get_charging_status();

#ifdef __cplusplus
}
#endif

#endif
