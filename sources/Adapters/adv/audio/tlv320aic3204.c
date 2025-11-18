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
#include "tim.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

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
enum TLVInput { NONE, MIC, LINEIN } input = NONE;

static volatile char overrideSpkr = 0;

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

#define LINEINBASEHALFDB 12
#define GAINMASK 0x3F
#define PRESERVEMASK 0xC0
#define MIC_GAIN_REGISTER 0x53 // Left ADC digital gain

static uint8_t clamp_half_db(int halfDb) {
  if (halfDb < 0) {
    return 0;
  }
  if (halfDb > GAINMASK) {
    return GAINMASK;
  }
  return (uint8_t)halfDb;
}

static void tlv320_update_gain_register(uint8_t reg, uint8_t baseHalfDb,
                                        int gainDb) {
  uint8_t current = 0;
  if (tlv320read(0x01, reg, &current) != HAL_OK) {
    current = 0;
  }
  const uint8_t desiredHalfDb = (uint8_t)baseHalfDb + gainDb * 2;
  const uint8_t halfDbValue = clamp_half_db(desiredHalfDb);
  const uint8_t updated = (current & PRESERVEMASK) | (halfDbValue & GAINMASK);
  tlv320writepage(0x01, reg, updated);
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

  // INIT INPUT
  // input frequency comes from MCU and is 11.2896MHz
  // Digital mic has the following specs:
  // Low‐Power Mode 400    800 kHz
  // Standard Mode 1.0    3.3 MHz
  // High‐Performance Mode 4.1    4.8 MHz
  // PDM frequency is OSR * ADC_FS
  // OSR is one of 32, 64, 128, 256
  // CLKIN = NADC * MADC * (A)OSR * ADC_FS
  //
  // restrictions:
  // ADC_FS = 44100Hz
  // CLKIN = 11.2896MHz
  // -> OSR = 64 to be in a valid range for the mic
  // -> PDM freq -> 64 X 44100Hz = 2.8224MHz (standard mode range)
  // -> NADC * MADC = 4
  // -> NADC = 1
  // -> MADC = 4
  // this has to be met: MADC * OSR / 32 > RC -> 8 > RC
  // -> Processing block: PRB_R1 (filter A)
  // Initialize to Page 0
  tlv320write(0, 0);
  // Power up NADC divider with value 1
  tlv320write(0x12, 0x81);
  // Power up MADC divider with value 4
  tlv320write(0x13, 0x84);
  // Program OSR for ADC to 64
  tlv320write(0x14, 0x40);
  // Select ADC PRB_R1
  tlv320write(0x3d, 0x01);

  // shouldn't be necessary but configuring dout explicitly
  tlv320write(0x35, 0x12);

  // MFP4 MIC CLOCK OUT
  tlv320write(0x37, 0x0e);
  // MFP5 MIC DATA IN
  tlv320write(0x34, 0x04);

  // Select Page 1
  tlv320write(0x00, 0x01);
  // Select ADC PTM_R4
  tlv320write(0x3d, 0x00);

  // Set MicPGA startup delay to 3.1ms
  tlv320write(0x47, 0x32);
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

  // Adjust the output amp for full volume to 1.4Vpp (0.5Vrms) for line level
  // Set the HPL gain to 2dB
  tlv320write(0x10, 0x02);
  // Set the HPR gain to 2dB
  tlv320write(0x11, 0x02);
  // Power up HPL and HPR drivers
  tlv320write(0x09, 0x30);
  // Wait for 2.5 sec for soft stepping to take effect
  // Else read Page 1, Register 63d, D(7 : 6).When = “11” soft - stepping is
  // complete
  // typical under 5ms
  vTaskDelay(pdMS_TO_TICKS(150));
  // Select Page 0
  tlv320write(0x00, 0x00);
  // Power up the Left and Right DAC Channels with route the Left Audio
  // digital data to Left Channel DAC and Right Audio digital data to
  // Right Channel DAC
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
  // typical under 5ms - up to 50ms on turn on
  vTaskDelay(pdMS_TO_TICKS(150));
  // Select Page 0
  tlv320write(0x00, 0x00);
  // Power up the Right DAC Channel and mix both data channels to right
  // dac (left disabled)
  tlv320write(0x3f, 0x4e);
}

void tlv320_select_output(void) {

  uint8_t value;
  tlv320read(0x00, 0x43, &value);

  if (value & 0x20 || input == MIC) {
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
  if (input == NONE) {
    input = LINEIN;
  } else {
    return;
  }
  // Select Page 1
  tlv320write(0x00, 0x01);
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
void tlv320_enable_mic(void) {
  if (input == NONE) {
    input = MIC;
  } else {
    return;
  }
  // Select Page 1
  tlv320write(0x00, 0x01);
  // Route IN2L to LEFT_P with 10K input impedance
  tlv320write(0x34, 0x10);
  // Route Common Mode to LEFT_M with impedance of 20K
  tlv320write(0x36, 0x02);

  // MICBIAS 2.5V
  // TODO: check this, we enable it for HP detect, shouldn't be needed here
  //  tlv320write(0x33, 0x60);

  // Unmute Left MICPGA, Gain selection of 6dB to make channel gain 0dB
  // Register of 6dB with input impedance of 20K = > Channel Gain of 0dB
  tlv320write(0x3b, 0xd0);

  // Select Page 0
  tlv320write(0x00, 0x00);

  // Power up Left ADC Channels // GPIO serves as Digital Microphone Input //
  // Left Channel ADC configured for Digital Microphone
  tlv320write(0x51, 0x88);
  // Unmute Left ADC Digital Volume Control.
  tlv320write(0x52, 0x08);

  // Recheck output in case Speaker needs to be mutted
  tlv320_select_output();
}

void tlv320_set_linein_gain_db(uint8_t gainDb) {
  if (input != LINEIN) {
    return;
  }
  tlv320_update_gain_register(0x3b, LINEINBASEHALFDB, gainDb);
  tlv320_update_gain_register(0x3c, LINEINBASEHALFDB, gainDb);
}

void tlv320_set_mic_gain_db(uint8_t gainDb) {
  if (input != MIC) {
    return;
  }
  // Register 0x53 encodes gain in 0.5dB steps with 0x00 = 0dB.
  const uint8_t regValue = gainDb * 2;
  tlv320writepage(0x00, MIC_GAIN_REGISTER, regValue);
}

void tlv320_disable_linein(void) {
  if (input == LINEIN) {
    input = NONE;
  } else {
    return;
  }
  // Select Page 0
  tlv320write(0x00, 0x00);
  // Mute Left and Right ADC Digital Volume Control.
  tlv320write(0x52, 0x88);
  // Power down Left and Right ADC Channels
  tlv320write(0x51, 0x00);

  // Select Page 1
  tlv320write(0x00, 0x01);
  // Mute Right MICPGA
  tlv320write(0x3c, 0x80);
  // Mute Left MICPGA
  tlv320write(0x3b, 0x80);
  // Unroute Right MICPGA
  tlv320write(0x39, 0x00);
  // Unroute IN1R to RIGHT_P
  tlv320write(0x37, 0x00);
  // Unroute Left MICPGA
  tlv320write(0x36, 0x00);
  // Unroute IN1L to LEFT_P
  tlv320write(0x34, 0x00);
}
void tlv320_disable_mic(void) {
  if (input == MIC) {
    input = NONE;
  } else {
    return;
  }
  // Select Page 0
  tlv320write(0x00, 0x00);
  // Mute Left ADC Digital Volume Control.
  tlv320write(0x52, 0x88);
  // Power down Left ADC Channels
  tlv320write(0x51, 0x00);

  // Select Page 1
  tlv320write(0x00, 0x01);
  // Mute Left MICPGA
  tlv320write(0x3b, 0x80);
  // Unroute Left MICPGA
  tlv320write(0x36, 0x00);
  // Route IN2L to LEFT_P
  tlv320write(0x34, 0x00);

  // Recheck output in case Speaker needs to be unmutted
  tlv320_select_output();
}

void tlv320_sleep(void) {
  // -- TABLE 1, STEP 1a: Configure amplifier gains to -6dB
  // w 30 00 01 # Switch to Page 1
  tlv320write(0x00, 0x01);
  // w 30 10 3A # HPL = -6dB, unmuted
  tlv320write(0x10, 0x3A);
  // w 30 11 3A # HPR = -6dB, unmuted
  tlv320write(0x11, 0x3A);
  // w 30 12 3A # LOL = -6dB, unmuted
  tlv320write(0x12, 0x3A);
  // w 30 13 3A # LOR = -6dB, unmuted
  tlv320write(0x13, 0x3A);
  // # f 30 3F 1111xxxx # Wait for p1_r63_b7-b4 to set
  //  tlv320write(0x3F, 0x);
  // # -- TABLE 1, STEP 1b: Power down internal amplifiers
  // w 30 10 7A # HPL = -6dB, muted
  tlv320write(0x10, 0x7A);
  // w 30 11 7A # HPR = -6dB, muted
  tlv320write(0x11, 0x7A);
  // w 30 12 7A # LOL = -6dB, muted
  tlv320write(0x12, 0x7A);
  // w 30 13 7A # LOR = -6dB, muted
  tlv320write(0x13, 0x7A);
  // w 30 09 00 # Power off HP/LO/MA amps
  tlv320write(0x09, 0x00);
  // w 30 00 00 # Switch to Page 0
  tlv320write(0x00, 0x00);
  // # f 30 25 x00xx00x # Wait for p0_r37_b6-5/2-1 to clear
  //  tlv320write(0x, 0x);
  // # -- TABLE 1, STEP 1c: Configure MicPGA
  // w 30 00 01 # Switch to Page 1
  tlv320write(0x00, 0x01);
  // w 30 3B 00 # Set MicPGA_L Gain D7 = 0
  tlv320write(0x3B, 0x00);
  // w 30 3C 00 # Set MicPGA_R Gain D7 = 0
  tlv320write(0x3C, 0x00);
  // # -- TABLE 1, STEP 2a: Set reference to automatic mode
  // w 30 7b 01 # Set the REF charging time to 40ms (automatic)
  tlv320write(0x7B, 0x01);
  // # -- TABLE 1, STEP 3: Disable AGCs
  // w 30 00 00 # Switch to Page 0
  tlv320write(0x00, 0x00);
  // w 30 57 00 # Disable LAGC noise gate
  tlv320write(0x57, 0x00);
  // w 30 56 00 # Disable LAGC
  tlv320write(0x56, 0x00);
  // w 30 5f 00 # Disable RAGC noise gate
  tlv320write(0x5f, 0x00);
  // w 30 5e 00 # Disable RAGC
  tlv320write(0x5E, 0x00);
  // # -- TABLE 1, STEP 4: Power off ADCs
  // w 30 51 00 # Power off LADC/RADC
  tlv320write(0x51, 0x00);
  // # f 30 24 x0xxx0xx # Wait for p0_r36_b6/b2 to clear
  //  tlv320write(0x, 0x);
  // # -- TABLE 1, STEP 5: Power off DACs
  // w 30 3F 14 # Power off LDAC/RDAC
  tlv320write(0x3F, 0x14);
  // # f 30 25 0xxx0xxx # Wait for p0_r37_b7/3 to clear
  //  tlv320write(0x, 0x);
  // # -- TABLE 1, STEP 6: Disconnect all output amplifier routings
  // w 30 00 01 # Switch to Page 1
  tlv320write(0x00, 0x01);
  // w 30 0C 00 # Disconnect HPL routings
  tlv320write(0x0C, 0x00);
  // w 30 0D 00 # Disconnect HPR routings
  tlv320write(0x0D, 0x00);
  // w 30 0E 00 # Disconnect LOL routings
  tlv320write(0x0E, 0x00);
  // w 30 0F 00 # Disconnect LOR routings
  tlv320write(0x0F, 0x00);
  // # -- TABLE 1, STEP 7: Power off additional blocks
  // w 30 33 00 # Power off MICBIAS
  tlv320write(0x33, 0x00);
  // w 30 3A 00 # Disable weak input common mode
  tlv320write(0x3A, 0x00);
  // w 30 00 00 # Switch to Page 0
  tlv320write(0x00, 0x00);
  // w 30 1D 00 # Disable forced ASI output
  tlv320write(0x1D, 0x00);
  // w 30 1A 01 # Power down CDIV_CLKIN M divider
  tlv320write(0x1A, 0x01);
  // w 30 1E 01 # Power down BCLK N divider
  tlv320write(0x1E, 0x01);
  // w 30 43 00 # Disable headset detection
  tlv320write(0x43, 0x00);
  // # -- TABLE 1, STEP 8: Power off clock generation tree
  // w 30 13 08 # Power down MADC = 8
  tlv320write(0x13, 0x08);
  // w 30 0C 08 # Power down MDAC = 8
  tlv320write(0x0C, 0x08);
  // w 30 12 02 # Power down NADC = 2
  tlv320write(0x12, 0x02);
  // w 30 0B 02 # Power down NDAC = 2
  tlv320write(0x0B, 0x02);
  // w 30 05 11 # Power down PLL
  tlv320write(0x05, 0x11);
  // # -- TABLE 1, STEP 9a: Configure AVDD
  // w 30 00 01 # Switch to Page 1
  tlv320write(0x00, 0x01);
  // w 30 02 09 # Disable Master Analog Power Control (write 0x08 if using
  // external AVDD)
  tlv320write(0x02, 0x09);
  //  w 30 01 00 # Enable weak AVDD to DVDD connection w 30 02 08
  tlv320write(0x01, 0x00);
  // # Power down ALDO (skip if using external AVDD)
}
