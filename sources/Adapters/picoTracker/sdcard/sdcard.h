/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef SDCARD_H_
#define SDCARD_H_

// SD card driver for SdFat

class SdioConfig;
extern SdioConfig g_sd_sdio_config;
#define SD_CONFIG g_sd_sdio_config

#endif
