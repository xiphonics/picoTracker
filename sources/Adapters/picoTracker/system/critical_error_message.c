#include "critical_error_message.h"
#include "Adapters/picoTracker/display/mode0.h"
#include <System/Console/nanoprintf.h>
#include <string.h>

// show this message and basically crash, so only for critical errors where
// we need to show user a message and cannot continue until a reboot
void critical_error_message(const char *message, int guruId) {
  mode0_init();

  mode0_set_font_index(0);

  mode0_set_palette_color(15, 0x0000); // BLACK
  mode0_set_palette_color(14, 0xF800); // RED
  mode0_set_background(MODE0_GURU_BG);
  mode0_clear(MODE0_GURU_BG);
  mode0_set_foreground(MODE0_GURU_TXT);

  int msglen = strlen(message) < 30 ? strlen(message) : 30;
  char msgbuffer[32];
  char gurumsgbuffer[32];
  npf_snprintf(gurumsgbuffer, sizeof(gurumsgbuffer),
               "   Guru Meditation: 00000.%04d", guruId);

  int center = (32 - msglen) / 2;
  if (center > 0) {
    memset(&msgbuffer[1], ' ', center - 1);
    memcpy(&msgbuffer[center], message, msglen);
    memset(&msgbuffer[center + msglen], ' ', 32 - center - msglen - 1);
    msgbuffer[0] = '#';
    msgbuffer[31] = '#';
    gurumsgbuffer[0] = '#';
    gurumsgbuffer[31] = '#';
  }
  // halt
  for (;;) {
    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 32; x++) {
        mode0_set_cursor(x, y + 10);
        if (y == 0 || y == 3) {
          mode0_putc('#', false);
        } else if (y == 2) {
          mode0_putc(gurumsgbuffer[x], false);
        } else {
          mode0_putc(msgbuffer[x], false);
        }
      }
    }
    mode0_draw_changed();
    sleep_ms(1000);
    mode0_clear(MODE0_GURU_BG);

    // draw just the message
    for (int x = 1; x < 31; x++) {
      mode0_set_cursor(x, 11);
      mode0_putc(msgbuffer[x], false);
      mode0_set_cursor(x, 12);
      mode0_putc(gurumsgbuffer[x], false);
    }
    mode0_draw_changed();
    sleep_ms(1000);
  }
}
