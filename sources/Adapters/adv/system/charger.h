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

bool startCharging(void);
bool stopCharging(void);
void powerOff();

typedef enum {
  NOT_CHARGING,
  PRE_CHARGE,
  FAST_CHARGE,
  CHARGE_DONE
} ChargingStatus;

typedef enum {
  VBUS_OK,
  VBUS_KO,
  CHARGE_END,
  CHARGE_FAULT,
  UNKNOWN,
} ChargerInt;

ChargingStatus getChargingStatus();
ChargerInt chargerIntReason();

#ifdef __cplusplus
}
#endif

#endif
