/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _PICOTRACKERINPUT_H_
#define _PICOTRACKERINPUT_H_
#include "pico/stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BIT(n) (1 << (n))

typedef enum KEYPAD_BITS {
  KEY_LEFT = BIT(0),  //!< Keypad LEFT button.
  KEY_DOWN = BIT(1),  //!< Keypad DOWN button.
  KEY_RIGHT = BIT(2), //!< Keypad RIGHT button.
  KEY_UP = BIT(3),    //!< Keypad UP button.
  KEY_ALT = BIT(4),   //!< Keypad ALT button.
  KEY_EDIT = BIT(5),  //!< Keypad EDIT button.
  KEY_ENTER = BIT(6), //!< Keypad ENTER button.
  KEY_NAV = BIT(7),   //!< Keypad NAV button.
  KEY_START = BIT(8), //!< Keypad PLAY button.
} KEYPAD_BITS;

uint16_t scanKeys();

#ifdef __cplusplus
}
#endif

#endif