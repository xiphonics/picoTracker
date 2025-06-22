/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "input.h"
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
  return (~gpio_get_all() & 0x0001FF00) >> 8;
}
