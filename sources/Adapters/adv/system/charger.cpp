/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2025 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#include "charger.h"
#include "System/Console/Trace.h"
#include "gpio.h"
#include "i2c.h"
#include "platform.h"
#include "rtc.h"
#include "tlv320aic3204.h"

#include <cstdint>
#include <cstdio>
#include <stdlib.h>

#define BQ25601_I2C_ADDR 0x6B
#define BQ25601_CHARGING_REG 0x01
#define BQ25601_SHIPMODE_REG 0x07
#define BQ25601_STATUS_REG 0x08
#define BQ25601_FAULT_REG 0x09
#define BQ25601_PG_STAT 1 << 2

// Note: NOT_CHARGING enum is specifically made to be 0 (ie. false)
ChargingStatus getChargingStatus() {
  uint8_t reg_value = 0;
  HAL_StatusTypeDef status =
      HAL_I2C_Mem_Read(&hi2c4, BQ25601_I2C_ADDR << 1, BQ25601_STATUS_REG,
                       I2C_MEMADD_SIZE_8BIT, &reg_value, 1, HAL_MAX_DELAY);

  if (status != HAL_OK) {
    Trace::Error("GetCharginStatus: i2c read error: %i", status);
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

void powerOff() {
  tlv320_mute();

  // Disable wake-up timer before reconfiguring
  HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);

  // Clear flags wake-up timer and power related flags
  __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&hrtc, RTC_FLAG_WUTF);
  __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&hrtc, RTC_FLAG_WUTWF);

  PWR->WKUPCR = 0xFFFFFFFFU;
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

  // if powered on only shutdown processor
  uint8_t reg_value = 0;
  HAL_StatusTypeDef status =
      HAL_I2C_Mem_Read(&hi2c4, BQ25601_I2C_ADDR << 1, BQ25601_STATUS_REG,
                       I2C_MEMADD_SIZE_8BIT, &reg_value, 1, HAL_MAX_DELAY);

  if (status != HAL_OK) {
    Trace::Error("PowerOff: i2c write error: %i", status);
  }

  if (reg_value & BQ25601_PG_STAT) {
    // TODO: we could further optimize and go to deep sleep if battery is full
    Trace::Log("POWEROFF", "Sleep");

    HAL_GPIO_DeInit(POWER_GPIO_Port, POWER_Pin);
    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN2);
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU); // always clear before enabling again
    PWREx_WakeupPinTypeDef sPinParams = {
        .WakeUpPin = PWR_WAKEUP_PIN2,
        .PinPolarity = PWR_PIN_POLARITY_LOW, // Rising edge triggers wakeup
        .PinPull = PWR_PIN_NO_PULL // Pulldown to ensure low level before press
    };
    HAL_PWREx_EnableWakeUpPin(&sPinParams);
    // Turn off AMP
    HAL_GPIO_WritePin(AMP_EN_GPIO_Port, AMP_EN_Pin, GPIO_PIN_RESET);

    // turn off codec
    tlv320_sleep();
    // TODO: turn off osc

    // Wait until WUTWF is set (means wake-up timer can be configured)
    while (__HAL_RTC_WAKEUPTIMER_GET_FLAG(&hrtc, RTC_FLAG_WUTWF) == RESET)
      ;

    uint32_t wakeup_time = 300; // wake up every 5 minutes to check
    HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, wakeup_time,
                                RTC_WAKEUPCLOCK_CK_SPRE_16BITS);

    HAL_PWR_EnterSTANDBYMode();

  } else {
    Trace::Log("POWEROFF", "Deep sleep");
    // BATFET_DIS = 1 as well as: BATFET_RST_EN = 1, TMR2X_EN = 1
    uint8_t value = 0x64;
    HAL_StatusTypeDef status =
        HAL_I2C_Mem_Write(&hi2c4, BQ25601_I2C_ADDR << 1, BQ25601_SHIPMODE_REG,
                          I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
    if (status != HAL_OK) {
      Trace::Error("PowerOff: i2c write error: %i", status);
    }
  }
}

// TODO:
// use in future to configure max charging current and turn OTG off
void configureCharging(void) {
  uint8_t value = 0x1a;
  HAL_StatusTypeDef status =
      HAL_I2C_Mem_Write(&hi2c4, BQ25601_I2C_ADDR << 1, BQ25601_CHARGING_REG,
                        I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
  if (status != HAL_OK) {
    printf("i2c write error: %i\r\n", status);
  }
  HAL_GPIO_WritePin(CHARGER_OTG_GPIO_Port, CHARGER_OTG_Pin, GPIO_PIN_RESET);
}
