/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#ifndef _TLV320AIC3204_
#define _TLV320AIC3204_

// #define I2C hi2c4
#define CODEC_ADDR (0x18 << 1)
// #define CODEC_RESET_PORT GPIOC
// #define CODEC_RESET_PIN GPIO_PIN_13
#define I2C_MEMADD_SIZE_8BIT (0x00000001U)

#define REFERENCE_PWR_UP_SLOW BIT(2)
#define COMMON_MODE_09V BIT(6)

#ifdef __cplusplus
extern "C" {
#endif

void tlv320_init();

void tlv320_select_output(void);
void tlv320_unmute(void);
void tlv320_mute(void);
void tlv320_enable_linein(void);
void tlv320_enable_mic(void);
void tlv320_disable_linein(void);
void tlv320_disable_mic(void);
void tlv320_sleep(void);

#ifdef __cplusplus
}
#endif

#endif
