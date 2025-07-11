#ifndef _USB_UTILS_H_
#define _USB_UTILS_H_
#include "tusb.h"
void sendUSBMidiMessage(const uint8_t *midicmd, uint8_t len);
void handleUSBInterrupts();
#endif