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
  const char line[4] = {char_border_single_vertical, label,
                        char_border_single_vertical, 0};
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
  draw_button(13, 14, char_button_up, keys & KEY_UP);
  draw_button(16, 14, char_button_play, keys & KEY_START);
  draw_button(19, 14, char_button_edit, keys & KEY_EDIT);

  // Row 2: Left, Down, Right, Enter/Set
  draw_button(10, 17, char_button_left, keys & KEY_LEFT);
  draw_button(13, 17, char_button_down, keys & KEY_DOWN);
  draw_button(16, 17, char_button_right, keys & KEY_RIGHT);
  draw_button(19, 17, char_button_enter, keys & KEY_ENTER);

  // Row 3: Alt, Nav
  draw_button(13, 20, char_button_alt, keys & KEY_ALT);
  draw_button(16, 20, char_button_nav, keys & KEY_NAV);
}