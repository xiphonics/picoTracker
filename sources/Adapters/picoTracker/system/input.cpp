#include "input.h"
#include <stdio.h>

uint16_t scanKeys() {
#ifdef USB_REMOTE_UI
  // This reads a byte from USB serial input in non-blocking way, by using
  // param of 0 for immediate timeout and check for return of 255 for no
  // char read result ref: https://forums.raspberrypi.com/viewtopic.php?t=303964
  // the byte is expected to be sent by the remote UI client in the same bitmask
  // format as is returned by the picotracker hardware reading the gpio pins
  // group by the code below.
  // That bit mask is documented in the KEYPAD_BITS enum.
  char c = getchar_timeout_us(0);
  if (c != 255) {
    return c;
  }
#endif
  return (~gpio_get_all() & 0x0001FF00) >> 8;
}
