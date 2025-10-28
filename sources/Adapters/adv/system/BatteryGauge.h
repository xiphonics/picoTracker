/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#ifndef BATTERY_GAUGE
#define BATTERY_GAUGE

#include "advSystem.h"
#include <stdint.h>

// bq27441-G1 I2C address (7-bit address: 0x55)
#define BQ27441_I2C_ADDR 0x55

// Standard Command codes from bq27441-G1 datasheet
#define BQ27441_CMD_TEMP 0x02    // Command to read temperature
#define BQ27441_CMD_VOLTAGE 0x04 // Command to read battery voltage
#define BQ27441_CMD_CURRENT 0x10 // Command to read current
#define BQ27441_CMD_SOC 0x1C     // Command to read State of Charge

// Use max signed int16 to indicate error
#define CURRENT_READ_ERROR 0x7FFF

enum SOHT { SOH_NOT_VALID = 0, SOH_INSTANT, SOH_INITIAL, SOH_READY, SOH_ERROR };

// Battery gauge configuration
bool configureBatteryGauge();
uint8_t getBatterySOC();
uint32_t getBatteryVoltage();
int32_t getBatteryTemperature();
int16_t getBatteryCurrent();
SOHT getBatteryStateOfHealthType();
int16_t getBatteryStateOfHealth();

#endif
