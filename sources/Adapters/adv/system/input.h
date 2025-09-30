/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#ifndef _PICOTRACKERINPUT_H_
#define _PICOTRACKERINPUT_H_

#include "utils.h"
#include <cstdint>

typedef enum KEYPAD_BITS {
  KEY_LEFT = BIT(0),   //!< Keypad LEFT button.
  KEY_DOWN = BIT(1),   //!< Keypad DOWN button.
  KEY_RIGHT = BIT(2),  //!< Keypad RIGHT button.
  KEY_UP = BIT(3),     //!< Keypad UP button.
  KEY_ALT = BIT(4),    //!< Keypad ALT button.
  KEY_EDIT = BIT(5),   //!< Keypad EDIT button.
  KEY_ENTER = BIT(6),  //!< Keypad ENTER button.
  KEY_NAV = BIT(7),    //!< Keypad NAV button.
  KEY_PLAY = BIT(8),   //!< Keypad PLAY button.
  KEY_SELECT = BIT(9), //!< Keypad SELECT button.
  KEY_POWER = BIT(10), //!< Power button (active low on GPIOA, GPIO_PIN_2).
} KEYPAD_BITS;

uint16_t scanKeys();

#endif
