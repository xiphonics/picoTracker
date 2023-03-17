#ifndef _MODE2_H
#define _MODE2_H

#include "ili9341/ili9341.h"

extern uint16_t mode2_buffer[ILI9341_TFTWIDTH * ILI9341_TFTHEIGHT];

void mode2_init();
void mode2_clear();
void mode2_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                uint16_t color);
void mode2_render();

#endif
