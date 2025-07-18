/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "batteryCharger.h"
#include "System/Console/Trace.h"
#include "i2c.h"

#define I2C_TIMEOUT_DELAY_MS 100

bool configureCharger() {
  // Any registers not explicityly set remain in default mode

  HAL_StatusTypeDef status;
  uint8_t value;
  uint8_t reg;

  // Reset watchdog
  // WD_RST = 1 on reg 0x01
  // This sets the device in host mode, we need to periodically reset the
  // watchdog timer in order for it not to reset the device to default mode
  reg = 0x01;
  value = 0x5a;
  status =
      HAL_I2C_Mem_Write(&hi2c4, BQ25601_I2C_ADDR << 1, reg,
                        I2C_MEMADD_SIZE_8BIT, &value, 1, I2C_TIMEOUT_DELAY_MS);
  if (status != HAL_OK) {
    Trace::Error("CHARGER: i2c write error: %i\r\n", status);
    return false;
  }
  // Set boost current max to 500mA
  // Set charge max current to 1A
  reg = 0x02;
  value = 0x11;
  status =
      HAL_I2C_Mem_Write(&hi2c4, BQ25601_I2C_ADDR << 1, reg,
                        I2C_MEMADD_SIZE_8BIT, &value, 1, I2C_TIMEOUT_DELAY_MS);
  if (status != HAL_OK) {
    Trace::Error("CHARGER: i2c write error: %i\r\n", status);
    return false;
  };

  //
  reg = 0x05;
  // Watchdog timer is default of 40s
  // Set charger limit to 5h
  // Set thermal regulation threshold to 90C
  value = 0x99;
  status =
      HAL_I2C_Mem_Write(&hi2c4, BQ25601_I2C_ADDR << 1, 0x03,
                        I2C_MEMADD_SIZE_8BIT, &value, 1, I2C_TIMEOUT_DELAY_MS);
  if (status != HAL_OK) {
    Trace::Error("CHARGER: i2c write error: %i\r\n", status);
    return false;
  };
  Trace::Log("CHARGER", "configured correctly");
  return true;
}

bool setMask(uint8_t reg, uint8_t mask) {
  HAL_StatusTypeDef status;
  uint8_t value;
  // TODO: Should we retry this? what if it fails after even retrying? maybe
  // indicate something in UI regarding charging
  status =
      HAL_I2C_Mem_Read(&hi2c4, BQ25601_I2C_ADDR << 1, reg, I2C_MEMADD_SIZE_8BIT,
                       &value, 1, I2C_TIMEOUT_DELAY_MS);
  if (status != HAL_OK) {
    printf("read failed with code %i\r\n", status);
  } else {
    printf("Read value %i \r\n", value);
  }

  value |= mask;
  status =
      HAL_I2C_Mem_Write(&hi2c4, BQ25601_I2C_ADDR << 1, reg,
                        I2C_MEMADD_SIZE_8BIT, &value, 1, I2C_TIMEOUT_DELAY_MS);
  if (status != HAL_OK) {
    Trace::Error("CHARGER: i2c write error: %i\r\n", status);
    return false;
  }
  return true;
}

bool resetWatchdog() { return setMask(0x01, BQ25601_WD_RST); }

bool powerGood() {
  uint8_t value = 0;
  auto status =
      HAL_I2C_Mem_Read(&hi2c4, BQ25601_I2C_ADDR << 1, 0x08,
                       I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
  if (status != HAL_OK) {
    Trace::Error("CHARGER: read failed with code %i", status);
  } else {
    if (value && BQ25601_PG_STAT) {
      return true;
    }
  }
  return false;
}
void shipMode() {
  Trace::Log("BATTERY", "Enter ship mode");
  setMask(0x07, BQ25601_BATFET_DIS);
}
