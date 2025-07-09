#include "input.h"
#include "Trace.h"
#include "main.h"
#include "stm32h7xx_hal.h"
#include <stdio.h>

uint16_t scanKeys() {
#ifdef USB_REMOTE_UI_INPUT
  // This reads a byte from USB serial input in non-blocking way, by using
  // param of 0 for immediate timeout and check for return of 255 for no
  // char read result ref: https://forums.raspberrypi.com/viewtopic.php?t=303964
  // the byte is expected to be sent by the remote UI client in the same bitmask
  // format as is returned by the picotracker hardware reading the gpio pins
  // group by the code below.
  // That bit mask is documented in the KEYPAD_BITS enum.
  char c = getchar_timeout_us(0);
  if (c != 0xFF) {
    // how to encode a 9bit bitmask into a 7bit char? (8th bit is not usable
    // because 0xFF means no result) answer: use MIDI style encoding, if bitmask
    // > 0x40 then set bit 7 to 1 to indicate 2 chars needed and then the next
    // char contains the remaining 3 bits
    int16_t mask = c & 0x3F;
    if (c & 0xFE) {
      // get remaining 3 bits from reading the next char:
      c = getchar_timeout_us(0);
      if (c != 0xFF) {
        mask += (c & 0x7) << 6;
      } else {
        // TODO: error missing follow up char!
      }
    }
    return mask;
  }
#endif
  // TODO(stm): implement this
  //  return (~gpio_get_all() & 0x0001FF00) >> 8;
  uint16_t input = 0;
  input |= HAL_GPIO_ReadPin(INPUT_LEFT_GPIO_Port, INPUT_LEFT_Pin);
  input |= HAL_GPIO_ReadPin(INPUT_DOWN_GPIO_Port, INPUT_DOWN_Pin) << 1;
  input |= HAL_GPIO_ReadPin(INPUT_RIGHT_GPIO_Port, INPUT_RIGHT_Pin) << 2;
  input |= HAL_GPIO_ReadPin(INPUT_UP_GPIO_Port, INPUT_UP_Pin) << 3;
  input |= HAL_GPIO_ReadPin(INPUT_ALT_GPIO_Port, INPUT_ALT_Pin) << 4;
  input |= HAL_GPIO_ReadPin(INPUT_EDIT_GPIO_Port, INPUT_EDIT_Pin) << 5;
  input |= HAL_GPIO_ReadPin(INPUT_ENTER_GPIO_Port, INPUT_ENTER_Pin) << 6;
  input |= HAL_GPIO_ReadPin(INPUT_NAV_GPIO_Port, INPUT_NAV_Pin) << 7;
  input |= HAL_GPIO_ReadPin(INPUT_PLAY_GPIO_Port, INPUT_PLAY_Pin) << 8;

  // Power button is active low on GPIOA, GPIO_PIN_2
  // Invert the reading so 1 means pressed (consistent with other buttons)
  input |= (!HAL_GPIO_ReadPin(POWER_GPIO_Port, POWER_Pin)) << 10;

  // if (input & (1 << 10)) {
  //   Trace::Debug("input pwr: %i", input);
  // }
  return input;
}
