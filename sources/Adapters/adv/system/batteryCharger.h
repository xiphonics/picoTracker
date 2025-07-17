/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#ifndef BATTERY_CHARGER_H
#define BATTERY_CHARGER_H

#include "advSystem.h"
#include <stdint.h>

#define BQ25601_I2C_ADDR 0x6B
#define BQ25601_WD_RST 0x40
#define BQ25601_BATFET_DIS 0x20
#define BQ25601_PG_STAT 0x04
/*
// Standard Command codes from bq27441-G1 datasheet
#define BQ27441_CMD_TEMP 0x02    // Command to read temperature
#define BQ27441_CMD_VOLTAGE 0x04 // Command to read battery voltage
#define BQ27441_CMD_CURRENT 0x10 // Command to read current
#define BQ27441_CMD_SOC 0x1C     // Command to read State of Charge

// Use max signed int16 to indicate error
#define CURRENT_READ_ERROR 0x7FFF
*/

bool configureCharger();

bool resetWatchdog();
bool powerGood();
void shipMode();
#endif
