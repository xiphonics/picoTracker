#include "usb_utils.h"

void sendUSBMidiMessage(uint8_t const *midicmd, uint8_t len) {
  tud_midi_n_stream_write(0, 0, midicmd, len);
};
