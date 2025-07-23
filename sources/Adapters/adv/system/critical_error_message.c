/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#include "critical_error_message.h"
#include "Adapters/adv/display/display.h"
#include "stm32h7xx_hal.h"
#include <System/Console/nanoprintf.h>
#include <string.h>

// show this message and basically crash, so only for critical errors where
// we need to show user a message and cannot continue until a reboot
void critical_error_message(const char *message, int guruId) {
  display_set_font_index(0);

  display_set_palette_color(15, 0x0000); // BLACK
  display_set_palette_color(14, 0xF800); // RED
  display_set_background(COLOR_GURU_BG);
  display_clear(COLOR_GURU_BG);
  display_set_foreground(COLOR_GURU_TXT);

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
        display_set_cursor(x, y + 10);
        if (y == 0 || y == 3) {
          display_putc('#', false);
        } else if (y == 2) {
          display_putc(gurumsgbuffer[x], false);
        } else {
          display_putc(msgbuffer[x], false);
        }
      }
    }
    display_draw_changed();
    HAL_Delay(1000);
    display_clear(COLOR_GURU_BG);

    // draw just the message
    for (int x = 1; x < 31; x++) {
      display_set_cursor(x, 11);
      display_putc(msgbuffer[x], false);
      display_set_cursor(x, 12);
      display_putc(gurumsgbuffer[x], false);
    }
    display_draw_changed();
    HAL_Delay(1000);
  }
}
