/**
 * Copyright (c) 2011-2024 Bill Greiman
 * This file is part of the SdFat library for SD memory cards.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#pragma once
#include "../SdCardInfo.h"
#include "../SdioCard.h"

// SDIO clock will be CPU_SPEED/(4*clkDiv)
// Use slightly slower clock for init of SD.
#define PIO_CLK_DIV_INIT 2

#define PIO_CLK_DIV_RUN 1

#define PIN_SDIO_UNDEFINED (31u)

#if defined(ARDUINO_ADAFRUIT_METRO_RP2040)
#define PIN_SDIO_CLK (18u)
#define PIN_SDIO_CMD (19u)
#define PIN_SDIO_DAT0 (20u)
#endif  //  ARDUINO_ADAFRUIT_METRO_RP2040

#if defined(ARDUINO_RASPBERRY_PI_PICO)
#define PIN_SDIO_CLK (16u)
#define PIN_SDIO_CMD (17u)
#define PIN_SDIO_DAT0 (18u)
#endif  // ARDUINO_RASPBERRY_PI_PICO

#if !defined(PIN_SDIO_CLK) || !defined(PIN_SDIO_CMD) || !defined(PIN_SDIO_DAT0)
// #error "PIO SDIO pins not defined"
#endif

/* Symbols for RP2040 variants from rp2040/hardware/rp2040/3.7.2/boards.txt
ARDUINO_0XCB_HELIOS
ARDUINO_ADAFRUIT_FEATHER_RP2040
ARDUINO_ADAFRUIT_FEATHER_RP2040_CAN
ARDUINO_ADAFRUIT_FEATHER_RP2040_DVI
ARDUINO_ADAFRUIT_FEATHER_RP2040_PROP_MAKER
ARDUINO_ADAFRUIT_FEATHER_RP2040_RFM
ARDUINO_ADAFRUIT_FEATHER_RP2040_SCORPIO
ARDUINO_ADAFRUIT_FEATHER_RP2040_THINKINK
ARDUINO_ADAFRUIT_FEATHER_RP2040_USB_HOST
ARDUINO_ADAFRUIT_ITSYBITSY_RP2040
ARDUINO_ADAFRUIT_KB2040_RP2040
ARDUINO_ADAFRUIT_MACROPAD_RP2040
ARDUINO_ADAFRUIT_METRO_RP2040
ARDUINO_ADAFRUIT_QTPY_RP2040
ARDUINO_ADAFRUIT_STEMMAFRIEND_RP2040
ARDUINO_ADAFRUIT_TRINKEYQT_RP2040
ARDUINO_ARTRONSHOP_RP2_NANO
ARDUINO_BRIDGETEK_IDM2040-7A
ARDUINO_CHALLENGER_2040_LORA_RP2040
ARDUINO_CHALLENGER_2040_LTE_RP2040
ARDUINO_CHALLENGER_2040_NFC_RP2040
ARDUINO_CHALLENGER_2040_SDRTC_RP2040
ARDUINO_CHALLENGER_2040_SUBGHZ_RP2040
ARDUINO_CHALLENGER_2040_UWB_RP2040
ARDUINO_CHALLENGER_2040_WIFI_BLE_RP2040
ARDUINO_CHALLENGER_2040_WIFI_RP2040
ARDUINO_CHALLENGER_2040_WIFI6_BLE_RP2040
ARDUINO_CHALLENGER_NB_2040_WIFI_RP2040
ARDUINO_CONNECTIVITY_2040_LTE_WIFI_BLE_RP2040
ARDUINO_CYTRON_MAKER_NANO_RP2040
ARDUINO_CYTRON_MAKER_PI_RP2040
ARDUINO_CYTRON_MAKER_UNO_RP2040
ARDUINO_DATANOISETV_PICOADK
ARDUINO_DEGZ_SUIBO_RP2040
ARDUINO_DFROBOT_BEETLE_RP2040
ARDUINO_ELECTRONICCATS_HUNTERCAT_NFC
ARDUINO_EXTREMEELEXTRONICS_RC2040
ARDUINO_FLYBOARD2040_CORE
ARDUINO_GENERIC_RP2040
ARDUINO_ILABS_2040_RPICO32_RP2040
ARDUINO_MELOPERO_COOKIE_RP2040
ARDUINO_MELOPERO_SHAKE_RP2040
ARDUINO_NANO_RP2040_CONNECT
ARDUINO_NEKOSYSTEMS_BL2040_MINI
ARDUINO_NULLBITS_BIT_C_PRO
ARDUINO_PIMORONI_PGA2040
ARDUINO_PIMORONI_PLASMA2040
ARDUINO_PIMORONI_TINY2040
ARDUINO_RAKWIRELESS_RAK11300
ARDUINO_RASPBERRY_PI_PICO
ARDUINO_RASPBERRY_PI_PICO_W
ARDUINO_REDSCORP_RP2040_EINS
ARDUINO_REDSCORP_RP2040_PROMINI
ARDUINO_SEA_PICRO
ARDUINO_SEEED_INDICATOR_RP2040
ARDUINO_SEEED_XIAO_RP2040
ARDUINO_SILICOGNITION_RP2040_SHIM
ARDUINO_SOLDERPARTY_RP2040_STAMP
ARDUINO_SPARKFUN_PROMICRO_RP2040
ARDUINO_SPARKFUN_THINGPLUS_RP2040
ARDUINO_UPESY_RP2040_DEVKIT
ARDUINO_VIYALAB_MIZU_RP2040
ARDUINO_WAVESHARE_RP2040_LCD_0_96
ARDUINO_WAVESHARE_RP2040_LCD_1_28
ARDUINO_WAVESHARE_RP2040_ONE
ARDUINO_WAVESHARE_RP2040_PLUS
ARDUINO_WAVESHARE_RP2040_ZERO
ARDUINO_WIZNET_5100S_EVB_PICO
ARDUINO_WIZNET_5500_EVB_PICO
ARDUINO_WIZNET_WIZFI360_EVB_PICO
ARDUINO_YD_RP2040ICO
*/