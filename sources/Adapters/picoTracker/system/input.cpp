#include "input.h"
#include <stdio.h>

uint16_t scanKeys() {
  // use param of 0 for immediate timeout and check for return of 255 for no
  // char read result ref: https://forums.raspberrypi.com/viewtopic.php?t=303964
  char c = getchar_timeout_us(0);
  if (c != 255) {
    return c;
  }
  return (~gpio_get_all() & 0x0001FF00) >> 8;
}
