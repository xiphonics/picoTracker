#ifndef _MODE1_H_
#define _MODE1_H_

#define MAP_WIDTH 416
#define MAP_HEIGHT 32
// the Pico and ILI9341 are different endianness so we need to byte swap our
// 16-bit color values
#define SWAP_BYTES(color) ((uint16_t)(color >> 8) | (uint16_t)(color << 8))

/* Tiled graphics mode */

typedef struct {
  uint8_t mem[24];
} Tile;

typedef struct {
  uint8_t mem[24];
} Sprite;

extern uint8_t background[MAP_HEIGHT * MAP_WIDTH];
extern uint8_t bg_palette[MAP_HEIGHT * MAP_WIDTH];
extern uint8_t height_offset; // (maybe?) in map blocks (8 pixels)
extern uint16_t global_background;
extern uint16_t *palette[8];
extern uint16_t scroll_offset; // this is in 'map' pixels, not 'screen' pixels
extern Tile tiles[256];

void mode1_init();

Tile *tile_at(int x, int y);
void set_tile_at(uint8_t tile_id, uint8_t palette, int x, int y);
uint8_t get_tile_palette_at(uint8_t x, uint8_t y);
void draw_slice(int x, int width);
void draw_background();
void scroll_background(int amount);

#endif
