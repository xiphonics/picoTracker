/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#include "tlv320aic3204.h"
#include "gpio.h"
#include "i2c.h"
#include "stm32h7xx_hal.h"

HAL_StatusTypeDef tlv320write(uint16_t reg, uint8_t value) {
  // Write to the register
  return HAL_I2C_Mem_Write(&I2C, CODEC_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &value,
                           1, HAL_MAX_DELAY);
}

HAL_StatusTypeDef tlv320writepage(uint16_t page, uint16_t reg, uint8_t value) {
  HAL_StatusTypeDef status;
  // Select the page
  status = tlv320write(0x00, page);
  if (status != HAL_OK)
    return status; // Return error if page selection fails

  // Write to the register
  status = tlv320write(reg, value);

  return status;
}

HAL_StatusTypeDef tlv320read(uint8_t page, uint8_t reg, uint8_t *value) {
  HAL_StatusTypeDef status;
  // Select the page
  status = HAL_I2C_Mem_Write(&I2C, CODEC_ADDR, 0x00, I2C_MEMADD_SIZE_8BIT,
                             &page, 1, HAL_MAX_DELAY);
  if (status != HAL_OK)
    return status; // Return error if page selection fails

  // Read from the register
  status = HAL_I2C_Mem_Read(&I2C, CODEC_ADDR, reg, I2C_MEMADD_SIZE_8BIT, value,
                            1, HAL_MAX_DELAY);

  return status;
}

void tlv320_reset() {
  // First reset by pulling reset down
  // Ensure reset is set first, then reset and then leave set
  HAL_GPIO_WritePin(CODEC_RESET_GPIO_Port, CODEC_RESET_Pin, GPIO_PIN_SET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(CODEC_RESET_GPIO_Port, CODEC_RESET_Pin, GPIO_PIN_RESET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(CODEC_RESET_GPIO_Port, CODEC_RESET_Pin, GPIO_PIN_SET);
  HAL_Delay(1);
}

void tlv320_init() {
  // hardware reset device on start
  tlv320_reset();
  /*
Assumption
AVdd = 1.8V, DVdd = 1.8V
MCLK = 11.2896MHz (44.1KHz)
Ext C = 47uF
Based on C the wait time will change.
Wait time = N*Rpop*C + 4* Offset ramp time
Default settings used.
PLL Disabled
DOSR 128
  */
  // Initialize to Page 0
  tlv320write(0, 0);
  // Initialize the device through software reset
  tlv320write(1, 1);
  // Power up the NDAC divider with value 1
  tlv320write(0xB, 0x81);
  // Power up the MDAC divider with value 2
  tlv320write(0xC, 0x82);
  // Program the OSR of DAC to 128
  tlv320write(0xD, 0x00);
  tlv320write(0xE, 0x80);
  // Set the word length of Audio Interface to 16bits
  tlv320write(0x1b, 0x00);
  // Set the DAC Mode to PRB_P8
  tlv320write(0x3C, 0x08);
  // Select Page 1
  tlv320write(0x00, 0x01);
  // Disable Internal Crude AVdd in presence of external AVdd supply or before
  // powering up internal AVdd LDO
  tlv320write(0x01, 0x08);
  // Enable Master Analog Power Control
  //  TODO Check this
  tlv320write(0x02, 0x00);
  // Set the REF charging time to 40ms
  tlv320write(0x7b, 0x01);
  // HP soft stepping settings for optimal pop performance at power up Rpop used
  // is 6k with N = 6 and soft step = 20usec. This should work with 47uF
  // coupling capacitor.Can try N = 5, 6 or 7 time constants as well.Trade - off
  // delay vs “pop” sound.
  tlv320write(0x14, 0x25);
  // Set the Input Common Mode to 0.9V and Output Common Mode for Headphone to
  // Input Common Mode
  tlv320write(0x0a, 0x00);
  // Route Left DAC to HPL
  tlv320write(0x0c, 0x08);
  // Route Right DAC to HPR
  tlv320write(0x0d, 0x08);
  // Set the DAC PTM mode to PTM_P3 / 4
  tlv320write(0x03, 0x00);
  tlv320write(0x04, 0x00);
  // Set the HPL gain to 0d
  tlv320write(0x10, 0x00);
  // Set the HPR gain to 0dB
  tlv320write(0x11, 0x00);
  // Power up HPL and HPR drivers
  tlv320write(0x09, 0x30);
  // Wait for 2.5 sec for soft stepping to take effect
  // Else read Page 1, Register 63d, D(7 : 6).When = “11” soft - stepping is
  // complete
  HAL_Delay(2500);
  // Select Page 0
  tlv320write(0x00, 0x00);
  // Power up the Left and Right DAC Channels with route the Left Audio digital
  // data to Left Channel DAC and Right Audio digital data to Right Channel DAC
  tlv320write(0x3f, 0xd6);
  // Unmute the DAC digital volume control
  tlv320write(0x40, 0x00);
}
