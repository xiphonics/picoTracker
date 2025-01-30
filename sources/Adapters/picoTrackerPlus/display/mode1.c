#include "mode1.h"
#include "ili9341.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

Sprite sprites[256];

uint8_t background[MAP_HEIGHT * MAP_WIDTH] = {0};
uint8_t bg_palette[MAP_HEIGHT * MAP_WIDTH] = {0};
uint16_t scroll_offset;    // this is in 'map' pixels, not 'screen' pixels
uint8_t height_offset = 0; // (maybe?) in map blocks (8 pixels)
uint16_t *palette[8];

void mode1_init() { ili9341_init(); }

/* SPRITES */

void draw_sprite(Sprite *sprite, uint16_t x, uint16_t y) {
  // TODO
}

void erase_sprite(Sprite *sprite, uint16_t x, uint16_t y) {
  // TODO
}

/* BACKGROUND */

Tile *tile_at(int x, int y) {
  uint8_t tile_id = background[y * MAP_WIDTH + x];
  return &tiles[tile_id];
}

void set_tile_at(uint8_t tile_id, uint8_t palette, int x, int y) {
  background[y * MAP_WIDTH + x] = tile_id;
  bg_palette[y * MAP_WIDTH + x] = palette;
}

uint8_t get_tile_palette_at(uint8_t x, uint8_t y) {
  return bg_palette[y * MAP_WIDTH + x];
}

void draw_background() { draw_slice(scroll_offset, SCREEN_WIDTH); }

void scroll_background(int amount) {

  scroll_offset += amount;
  uint16_t screen_offset = scroll_offset % SCREEN_WIDTH;

  ili9341_set_command(ILI9341_VSCRSADD);
  ili9341_command_param((uint8_t)(screen_offset / 256));
  ili9341_command_param((uint8_t)(screen_offset % 256));

  draw_slice(SCREEN_WIDTH + scroll_offset - amount, amount);
}

void draw_slice(int x, int width) {
  uint16_t
      buffer[width * SCREEN_HEIGHT]; // 'amount' pixels wide, 240 pixels tall
  int buffer_idx = 0;

  for (int h = 0; h < width; h++) {
    int map_x = (x + h) % (MAP_WIDTH * 8);
    int tile_x = map_x / 8;
    int tile_x_offset = map_x % 8;

    for (int tile_y = 29; tile_y >= 0; tile_y--) {
      Tile *tile = tile_at(tile_x, (tile_y + height_offset) % MAP_HEIGHT);
      // uint32_t foo = (tile->mem[0][tile_x_offset] << 16) |
      // (tile->mem[1][tile_x_offset] << 8) | | tile->mem[2][tile_x_offset]

      // the tile data is stored NES style, so we have to do some bit twiddling

      uint8_t tile_palette_idx =
          get_tile_palette_at(tile_x, (tile_y + height_offset) % MAP_HEIGHT);
      uint16_t *tile_palette = palette[tile_palette_idx];

      for (int i = 0; i < 8; i++) {
        uint8_t palette_index = 0;
        for (int bit = 2; bit >= 0; bit--) {
          palette_index <<= 1;
          palette_index |=
              ((tile->mem[bit * 8 + (7 - i)] >> (7 - tile_x_offset)) & 1);
        }
        // look up the color from the palette
        // color 0 is "transparent", will show global background color instead
        uint16_t color =
            palette_index ? tile_palette[palette_index] : global_background;

        // this color is pre-byteswapped and blue-red swapped
        buffer[buffer_idx++] = color;
      }
    }
  }

  // set the address to write to
  // page address set

  // TODO to write a full screen when the scroll_offset is set, we actually
  // need to write starting at page 0 -- in other words, according to the LCD's
  // memory layout, not starting at the VSCRADD scroll value
  uint16_t write_start = (scroll_offset - width) % SCREEN_WIDTH;
  uint16_t write_stop = (write_start + width - 1) % SCREEN_WIDTH;
  ili9341_set_command(ILI9341_PASET);
  ili9341_command_param(write_start / 256);
  ili9341_command_param(write_start % 256); // start page
  ili9341_command_param(write_stop / 256);
  ili9341_command_param(write_stop % 256); // end page -> 319

  // write out this data
  ili9341_set_command(ILI9341_RAMWR);
  ili9341_write_data(buffer, width * SCREEN_HEIGHT * 2);
}
