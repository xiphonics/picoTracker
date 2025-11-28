/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "advRemoteUI.h"
#include "tusb.h"
#include <Adapters/adv/platform/platform.h>
#include <cstdint>

// The Remote UI protocol consists of sending ASCII messages over the USB serial
// connection to a client to render the UI shown by the picotracker and then
// listening for incoming button input events sent by the remote client.
//
// The messages consist of the types in the RemoteUICommand enum below and
// essentially just relay the existing draw, clear and setcolor commands that
// already exist in picTrackerGUIWidowImp.cpp
//
// Remote UI clients should pay careful attention to using the
// REMOTE_UI_CMD_MARKER and also verify the expected byte count length of each
// command received to check for any transmission errors.
void sendToUSBCDC(char data[], uint32_t length) {
  if (!tud_cdc_connected()) {
    return;
  }

  uint32_t sent = 0;
  while (sent < length) {
    uint32_t available = tud_cdc_write_available();
    if (available == 0) {
      // Give TinyUSB time to flush TX FIFO via its own USBDevice task.
      vTaskDelay(pdMS_TO_TICKS(1));
      continue;
    }

    uint32_t to_send = length - sent;
    if (to_send > available) {
      to_send = available; // Only send what fits
    }
    // tud_cdc_write() returns the number of bytes actually written
    uint32_t written = tud_cdc_write(data + sent, to_send);
    if (written == 0) {
      // Avoid a tight loop if TinyUSB temporarily reports no space
      vTaskDelay(pdMS_TO_TICKS(1));
      continue;
    }
    sent += written;
  }

  // After the entire message is queued, flush it.
  tud_cdc_write_flush();
}
