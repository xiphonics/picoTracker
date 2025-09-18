/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "picoRemoteUI.h"
#include "tusb.h"
#include <cstdint>

void sendToUSBCDC(char buf[], int length) {
  // based on PICO SDk's USB STDIO stdio_usb_out_chars function
  // https://github.com/raspberrypi/pico-sdk/blob/master/src/rp2_common/pico_stdio_usb/stdio_usb.c#L101
  static uint64_t last_avail_time;
  if (tud_cdc_connected()) {
    for (int i = 0; i < length;) {
      int n = length - i;
      int avail = (int)tud_cdc_write_available();
      if (n > avail)
        n = avail;
      if (n) {
        int n2 = (int)tud_cdc_write(buf + i, (uint32_t)n);
        tud_task();
        tud_cdc_write_flush();
        i += n2;
        last_avail_time = time_us_64();
      } else {
        tud_task();
        tud_cdc_write_flush();
        if (!tud_cdc_connected() ||
            (!tud_cdc_write_available() &&
             time_us_64() > last_avail_time + USB_TIMEOUT_US)) {
          break;
        }
      }
    }
  }
}
