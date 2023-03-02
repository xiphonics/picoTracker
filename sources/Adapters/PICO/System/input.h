#ifndef _PICO_INPUT_H_
#define _PICO_INPUT_H_
#include "pico/stdlib.h"

#define BIT(n) (1 << (n))

typedef enum KEYPAD_BITS {
  KEY_LEFT = BIT(0),   //!< Keypad LEFT button.
  KEY_DOWN = BIT(1),   //!< Keypad DOWN button.
  KEY_RIGHT = BIT(2),  //!< Keypad RIGHT button.
  KEY_UP = BIT(3),     //!< Keypad UP button.
  KEY_L = BIT(4),      //!< Left shoulder button.
  KEY_B = BIT(5),      //!< Keypad B button.
  KEY_A = BIT(6),      //!< Keypad A button.
  KEY_R = BIT(7),      //!< Right shoulder button.
  KEY_START = BIT(8),  //!< Keypad START button.
  KEY_SELECT = BIT(9), //!< Keypad SELECT button.
} KEYPAD_BITS;

void inputInit();
uint16_t scanKeys();

#endif
