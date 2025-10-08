/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#include "critical_error_message.h"
#include "Adapters/adv/display/display.h"
#include "power.h"
#include "stm32h7xx_hal.h"
#include "tim.h"
#include <System/Console/nanoprintf.h>
#include <string.h>

// This function assumes TIM2 is configured to have a 1MHz clock (1us tick)
// Need to use this instead of HAL_Delay() as critical_error_message() can get
// called in early boot before time for Hal_Delay is configured
static void delay_us(uint32_t us) {
  uint32_t start = __HAL_TIM_GET_COUNTER(&htim2);
  while ((__HAL_TIM_GET_COUNTER(&htim2) - start) < us) {
    // Loop until the desired number of microseconds have passed.
    // This handles counter wrapping correctly because of unsigned arithmetic.
  }
}

static void delay_ms(uint32_t ms) { delay_us(ms * 1000); }

// show this message and then power down after the given delay (in seconds) so
// only for critical errors where we need to show user a message and cannot
// continue until a reboot
void critical_error_message(const char *message, int guruId, int shutdownDelay,
                            bool showGuru) {
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
    for (int y = 0; y < (showGuru == true ? 4 : 3); y++) {
      for (int x = 0; x < 32; x++) {
        display_set_cursor(x, y + 10);
        if (y == 0 || y == (showGuru == true ? 3 : 2)) {
          display_putc('#', false);
        } else if (y == 2) {
          if (showGuru) {
            display_putc(gurumsgbuffer[x], false);
          }
        } else {
          display_putc(msgbuffer[x], false);
        }
      }
    }
    display_draw_changed();
    delay_ms(1000);
    if (shutdownDelay-- < 0) {
      power_off();
    }
    display_clear(COLOR_GURU_BG);

    // draw just the message
    for (int x = 1; x < 31; x++) {
      display_set_cursor(x, 11);
      display_putc(msgbuffer[x], false);
      display_set_cursor(x, 12);
      if (showGuru) {
        display_putc(gurumsgbuffer[x], false);
      } else {
        display_putc(' ', false);
      }
    }
    display_draw_changed();
    delay_ms(1000);
    if (shutdownDelay-- < 0) {
      // power_off();
    }
  }
}