#include "mode2.h"
#include <string.h>

#define SIZE (ILI9341_TFTHEIGHT * ILI9341_TFTWIDTH)

uint16_t mode2_buffer[SIZE] = {0};

void mode2_init() {}

void mode2_clear() { memset(mode2_buffer, 0, SIZE * sizeof(uint16_t)); }

void mode2_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                uint16_t color) {
  uint16_t *base_loc = &mode2_buffer[x * ILI9341_TFTWIDTH + y];

  for (int h = 0; h < width; h++) {
    uint16_t *loc = base_loc + h * ILI9341_TFTWIDTH;
    for (int v = 0; v < height; v++) {
      *loc++ = color;
    }
  }
}

void mode2_render() {
  ili9341_start_writing();
  ili9341_write_data_continuous(mode2_buffer, SIZE * sizeof(uint16_t));
  ili9341_stop_writing();
}
