#include "critical_error_message.h"
#include "Adapters/picoTracker/display/mode0.h"

const char msg_on[4][32] = {{"################################"},
                            {"#       SDCARD NOT FOUND       #"},
                            {"################################"}};
const char msg_off[4][32] = {{"                                "},
                             {"        SDCARD NOT FOUND        "},
                             {"                                "}};

// show this message and basically crash, so only for critical errors where
// we need to show user a message and cannot continue until a reboot
void critical_error_message() {
  mode0_init();

  mode0_set_font_index(0);

  mode0_set_palette_color(15, 0x0000); // BLACK
  mode0_set_palette_color(14, 0xF800); // RED
  mode0_set_background(MODE0_GURU_BG);
  mode0_clear(MODE0_GURU_BG);
  mode0_set_foreground(MODE0_GURU_TXT);

  // halt
  for (;;) {
    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 32; x++) {
        mode0_set_cursor(x, y + 10);
        mode0_putc(msg_on[y][x], false);
      }
    }
    mode0_draw_changed();

    sleep_ms(1000);
    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 32; x++) {
        mode0_set_cursor(x, y + 10);
        mode0_putc(msg_off[y][x], false);
      }
    }
    mode0_draw_changed();
    sleep_ms(1000);
  };
}