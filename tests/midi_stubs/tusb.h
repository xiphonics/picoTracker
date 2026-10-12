#pragma once
#include <cstdint>
bool tud_midi_mounted();
uint32_t tud_midi_available();
bool tud_midi_packet_read(uint8_t *packet);
