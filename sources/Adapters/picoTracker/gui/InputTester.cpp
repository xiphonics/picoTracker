#include "InputTester.h"
#include "Adapters/picoTracker/display/chargfx.h"
#include "Adapters/picoTracker/system/input.h"
#include "Foundation/Constants/SpecialCharacters.h"
#include <System/Console/nanoprintf.h>
#include <string.h>

// Helper function to draw a single button with optional inversion
static void draw_string(int x, int y, const char *s, bool invert) {
  while (*s) {
    chargfx_set_cursor(x++, y);
    chargfx_putc(*s++, invert);
  }
}

static void draw_button(uint8_t x, uint8_t y, char label, bool invert) {
  const char line[4] = {GLYPH(char_border_single_vertical_s), label,
                        GLYPH(char_border_single_vertical_s), 0};
  draw_string(x, y + 0,
              char_border_single_topLeft_s char_border_single_horizontal_s
                  char_border_single_topRight_s,
              invert);
  draw_string(x, y + 1, (const char *)line, invert);
  draw_string(x, y + 2,
              char_border_single_bottomLeft_s char_border_single_horizontal_s
                  char_border_single_bottomRight_s,
              invert);
}

void drawInputTester() {
  // Read button state
  uint16_t keys = scanKeys();

  // Title
  chargfx_set_cursor(10, 2);
  chargfx_print("Button Tester", false);

  // Row 1: Up, Play, Edit
  draw_button(13, 14, GLYPH(char_button_up_s), keys & KEY_UP);
  draw_button(16, 14, GLYPH(char_button_play_s), keys & KEY_START);
  draw_button(19, 14, GLYPH(char_button_edit_s), keys & KEY_EDIT);

  // Row 2: Left, Down, Right, Enter/Set
  draw_button(10, 17, GLYPH(char_button_left_s), keys & KEY_LEFT);
  draw_button(13, 17, GLYPH(char_button_down_s), keys & KEY_DOWN);
  draw_button(16, 17, GLYPH(char_button_right_s), keys & KEY_RIGHT);
  draw_button(19, 17, GLYPH(char_button_enter_s), keys & KEY_ENTER);

  // Row 3: Alt, Nav
  draw_button(13, 20, GLYPH(char_button_alt_s), keys & KEY_ALT);
  draw_button(16, 20, GLYPH(char_button_nav_s), keys & KEY_NAV);
}
