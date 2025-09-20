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
#include <stdio.h>

#include "usart.h"
#ifdef __GNUC__ /* __GNUC__ */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

PUTCHAR_PROTOTYPE {
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART3 and Loop until the end of
             transmission */
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0x000F);
  return ch;
}

enum TLVOutput { INIT, HP, SPKR } output = INIT;

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

  // configure HP detect
  // Configure MFP3 GPIO disabled
  tlv320write(0, 0);
  tlv320write(0x38, 0x0);
  // enable MICBIAS
  tlv320write(0x33, 0x40);
  // Configure headset detect (64ms debounce time)
  tlv320write(0x43, 0x88);

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
}

void tlv320_enable_hp(void) {
  // Select Page 1
  tlv320write(0x00, 0x01);
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
}

void tlv320_enable_spkr(void) {

  // Select Page 1
  tlv320write(0x00, 0x01);
  // Route Right DAC negative to LOL
  tlv320write(0x0e, 0x10);
  // Route Right DAC to LOR
  tlv320write(0x0f, 0x08);

  // Set the DAC PTM mode to PTM_P3 / 4
  tlv320write(0x03, 0x00);
  tlv320write(0x04, 0x00);

  // Set the LOL gain to 0dB
  tlv320write(0x12, 0x00);
  // Set the LOR gain to 0dB
  tlv320write(0x13, 0x00);

  // Power up LOL and LOR drivers
  tlv320write(0x09, 0x0c);
  // Wait for 2.5 sec for soft stepping to take effect
  // Else read Page 1, Register 63d, D(7 : 6).When = “11” soft - stepping is
  // complete
  HAL_Delay(2500);
  // Select Page 0
  tlv320write(0x00, 0x00);
  // Power up the Right DAC Channel and mix both data channels to right dac
  // (left disabled)
  tlv320write(0x3f, 0x4e);
}

void tlv320_select_output(void) {
  uint8_t value;
  tlv320read(0x00, 0x43, &value);

  if (value & 0x20) {
    if (output != HP) {
      tlv320_enable_hp();
      output = HP;
    }
  } else {
    if (output != SPKR) {
      tlv320_enable_spkr();
      output = SPKR;
    }
  }
}

void tlv320_mute(void) {
  // Select Page 0
  tlv320write(0x00, 0x00);
  uint8_t value;
  tlv320read(0x00, 0x40, &value);

  value |= 0x0C;
  // Mute the DAC digital volume control
  tlv320write(0x40, value);
}

void tlv320_unmute(void) {
  // Select Page 0
  tlv320write(0x00, 0x00);
  uint8_t value;
  tlv320read(0x00, 0x40, &value);

  value &= ~(1 << 2);
  value &= ~(1 << 3);
  // unmute the DAC digital volume control
  tlv320write(0x40, value);
}

void tlv320_enable_linein(void) {

  // Initialize to Page 0
  tlv320write(0, 0);
  // Power up NADC divider with value 1
  tlv320write(0x12, 0x81);
  // Power up MADC divider with value 2
  tlv320write(0x13, 0x82);
  // Program OSR for ADC to 128
  tlv320write(0x14, 0x80);
  // Select ADC PRB_R1
  tlv320write(0x3d, 0x01);

  // shouldn't be necessary but configuring dout explicitly
  tlv320write(0x35, 0x12);

  // Select Page 1
  tlv320write(0x00, 0x01);
  // Select ADC PTM_R4
  tlv320write(0x3d, 0x00);
  // Set MicPGA startup delay to 3.1ms
  tlv320write(0x47, 0x32);
  // Route IN1L to LEFT_P with 20K input impedance
  tlv320write(0x34, 0x80);
  // Route Common Mode to LEFT_M with impedance of 20K
  tlv320write(0x36, 0x80);
  // Route IN1R to RIGHT_P with input impedance of 20K
  tlv320write(0x37, 0x80);
  // Route Common Mode to RIGHT_M with impedance of 20K
  tlv320write(0x39, 0x80);
  // Unmute Left MICPGA, Gain selection of 6dB to make channel gain 0dB
  // Register of 6dB with input impedance of 20K = > Channel Gain of 0dB
  tlv320write(0x3b, 0x0c);
  // Unmute Right MICPGA, Gain selection of 6dB to make channel gain 0dB
  // Register of 6dB with input impedance of 20K = > Channel Gain of 0dB
  tlv320write(0x3c, 0x0c);
  // Select Page 0
  tlv320write(0x00, 0x00);
  // Power up Left and Right ADC Channels
  tlv320write(0x51, 0xc0);
  // Unmute Left and Right ADC Digital Volume Control.
  tlv320write(0x52, 0x00);
}
void tlv320_enable_mic(void) {}
void tlv320_disable_linein(void) {}
void tlv320_disable_mic(void) {}
