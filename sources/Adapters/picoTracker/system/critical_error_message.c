/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "critical_error_message.h"
#include "Adapters/picoTracker/display/chargfx.h"
#include "Foundation/Constants/SpecialCharacters.h"
#include <System/Console/nanoprintf.h>
#include <string.h>

// show this message and basically crash, so only for critical errors where
// we need to show user a message and cannot continue until a reboot
void critical_error_message(const char *message, int guruId,
                            bool (*externalCallback)(void)) {
  chargfx_init();

  chargfx_set_font_index(0);

  chargfx_set_palette_color(15, 0x0000); // BLACK
  chargfx_set_palette_color(14, 0xF800); // RED

  chargfx_set_background(CHARGFX_GURU_BG);
  chargfx_set_foreground(CHARGFX_GURU_TXT);

  int msglen = strlen(message) < 30 ? strlen(message) : 30;
  char msgbuffer[32];

  int center = (32 - msglen) / 2;
  if (center > 0) {
    memset(&msgbuffer[1], ' ', center - 1);
    memcpy(&msgbuffer[center], message, msglen);
    memset(&msgbuffer[center + msglen], ' ', 32 - center - msglen - 1);
  }

  chargfx_clear(CHARGFX_GURU_BG);

  // halt
  for (int i = 0;; i++) {
    bool border = i % 10 < 5;
    // prepare message lines with changing border
    msgbuffer[0] = border ? char_border_double_vertical : ' ';
    msgbuffer[31] = msgbuffer[0];

    chargfx_set_cursor(10, 2);
    chargfx_putc(' ', false);

    // draw border
    for (int y = 0; y < 3; y++) {
      for (int x = 0; x < 32; x++) {
        chargfx_set_cursor(x, y + 10);
        if (y == 0 || y == 2) {
          char c = char_border_double_horizontal;
          if (x == 0 || x == 31) {
            c = (x == 0) ? (y == 0 ? char_border_double_topLeft
                                   : char_border_double_bottomLeft)
                         : (y == 0 ? char_border_double_topRight
                                   : char_border_double_bottomRight);
          }
          chargfx_putc(border ? c : ' ', false);
        } else {
          chargfx_putc(msgbuffer[x], false);
        }
      }
    }

    chargfx_draw_changed();

    // call the external callback if provided to check if the error was cleared.
    if (externalCallback != NULL && externalCallback()) {
      return;
    }

    sleep_ms(100);
  }
}