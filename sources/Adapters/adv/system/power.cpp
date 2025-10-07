#include "power.h"
#include "gpio.h"
#include "i2c.h"
#include "platform.h"
#include "tlv320aic3204.h"

#include <cstdint>
#include <cstdio>
#include <stdlib.h>

void set_charging(void) {
  uint8_t value = 0x1a;
  HAL_StatusTypeDef status = HAL_I2C_Mem_Write(
      &hi2c4, 0x6b << 1, 0x01, I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
  if (status != HAL_OK) {
    printf("i2c write error: %i\r\n", status);
  }
  HAL_GPIO_WritePin(CHARGER_OTG_GPIO_Port, CHARGER_OTG_Pin, GPIO_PIN_RESET);
}

void power_off() {
  tlv320_mute();

  // Ship mode
  uint8_t value = 0x64;
  HAL_StatusTypeDef status = HAL_I2C_Mem_Write(
      &hi2c4, 0x6b << 1, 0x07, I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
  if (status != HAL_OK) {
    printf("i2c write error: %i\r\n", status);
  }

  set_charging();

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