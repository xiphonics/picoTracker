#include "input.h"

uint16_t scanKeys() { return (~gpio_get_all() & 0x0001FF00) >> 8; }
