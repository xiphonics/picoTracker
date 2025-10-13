/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2025 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#include "charger.h"
#include "gpio.h"
#include "i2c.h"
#include "platform.h"
#include "tlv320aic3204.h"

#include <cstdint>
#include <cstdio>
#include <stdlib.h>

#define BQ25601_I2C_ADDR 0x6B
#define BQ25601_STATUS_REG 0x08

ChargingStatus get_charging_status() {
  uint8_t reg_value = 0;
  HAL_StatusTypeDef status =
      HAL_I2C_Mem_Read(&hi2c4, BQ25601_I2C_ADDR << 1, BQ25601_STATUS_REG,
                       I2C_MEMADD_SIZE_8BIT, &reg_value, 1, HAL_MAX_DELAY);

  if (status != HAL_OK) {
    printf("i2c read error: %i\r\n", status);
    return NOT_CHARGING;
  }

  // Extract CHG_STAT bits (4 and 5)
  uint8_t chg_stat = (reg_value >> 4) & 0x03;

  switch (chg_stat) {
  case 0x00:
    return NOT_CHARGING;
  case 0x01:
    return PRE_CHARGE;
  case 0x02:
    return FAST_CHARGE;
  case 0x03:
    return CHARGE_DONE;
  default:
    return NOT_CHARGING; // Should not happen
  }
}

void power_off() {
  tlv320_mute();

  // Ship mode
  uint8_t value = 0x64;
  HAL_StatusTypeDef status =
      HAL_I2C_Mem_Write(&hi2c4, BQ25601_I2C_ADDR << 1, 0x07,
                        I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
  if (status != HAL_OK) {
    printf("i2c write error: %i\r\n", status);
  }

  HAL_GPIO_DeInit(POWER_GPIO_Port, POWER_Pin);
  HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN2);
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU); // always clear before enabling again
  PWREx_WakeupPinTypeDef sPinParams = {
      .WakeUpPin = PWR_WAKEUP_PIN2,
      .PinPolarity = PWR_PIN_POLARITY_LOW, // Rising edge triggers wakeup
      .PinPull = PWR_PIN_NO_PULL // Pulldown to ensure low level before press
  };
  HAL_PWREx_EnableWakeUpPin(&sPinParams);
  HAL_PWR_EnterSTANDBYMode();
}

// TODO:
// use in future to configure max charging current and turn OTG off
void configure_charging(void) {
  uint8_t value = 0x1a;
  HAL_StatusTypeDef status =
      HAL_I2C_Mem_Write(&hi2c4, BQ25601_I2C_ADDR << 1, 0x01,
                        I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
  if (status != HAL_OK) {
    printf("i2c write error: %i\r\n", status);
  }
  HAL_GPIO_WritePin(CHARGER_OTG_GPIO_Port, CHARGER_OTG_Pin, GPIO_PIN_RESET);
}