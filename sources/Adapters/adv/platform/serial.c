#include "main.h"
#include <stdint.h>

// Helper: converts a uint32_t to a Unicode string (USB style) as ASCII
static void int_to_ascii_hex(uint32_t value, char *out, int len) {
  for (int i = 0; i < len; i++) {
    uint8_t nibble = (value >> (28 - i * 4)) & 0xF;
    out[i] = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
  }
}

// Return device serial number
const char *platform_serial(void) {
  static char serial[13]; // 8 + 4 + null
  uint32_t deviceserial0 = *(uint32_t *)(UID_BASE);
  uint32_t deviceserial1 = *(uint32_t *)(UID_BASE + 0x04);
  uint32_t deviceserial2 = *(uint32_t *)(UID_BASE + 0x08);

  deviceserial0 += deviceserial2; // match USB DFU logic

  int_to_ascii_hex(deviceserial0, &serial[0], 8);
  int_to_ascii_hex(deviceserial1, &serial[8], 4);
  serial[12] = '\0';

  return serial;
}
