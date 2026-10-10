/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026 xiphonics, inc.
 */
#ifndef PICOTRACKER_SDFAT_CONFIG_H
#define PICOTRACKER_SDFAT_CONFIG_H

#include <stdint.h>

// Compile SdFat without Arduino, using the existing pico SDIO driver.
#define ENABLE_ARDUINO_FEATURES 0
#define ENABLE_ARDUINO_SERIAL 0
#define ENABLE_ARDUINO_STRING 0
#define SPI_DRIVER_SELECT 3
#define SD_CHIP_SELECT_MODE 2
#define ENABLE_DEDICATED_SPI 1
#define HAS_SDIO_CLASS 1
#define PICOTRACKER_SDIO 1
#define SS 0

#ifdef __cplusplus
// SdFat PrintBasic declares overloads using Arduino's opaque string type.
class __FlashStringHelper;
uint32_t millis();
#endif

#endif
