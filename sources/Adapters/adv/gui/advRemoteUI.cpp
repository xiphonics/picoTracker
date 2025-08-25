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
// Note that to avoid issues with non-printing chars being mishandled by either
// the pico SDK or client serial port drivers, most command param "values" have
// been attempted to be offset to push them into the printable char range.
//
// Remote UI clients should pay careful attention to using the
// REMOTE_UI_CMD_MARKER and also verify the expected byte count length of each
// command received to check for any transmission errors.
//

void sendToUSBCDC(char data[], uint32_t length) {
  if (!tud_cdc_connected()) {
    return;
  }

  uint32_t sent = 0;
  while (sent < length) {
    // tud_task() is called by the USB FreeRTOS task
    // to process outgoing data, freeing up buffer space

    uint32_t available = tud_cdc_write_available();
    if (available > 0) {
      uint32_t to_send = length - sent;
      if (to_send > available) {
        to_send = available; // Only send what fits
      }

      // tud_cdc_write() returns the number of bytes actually written
      uint32_t written = tud_cdc_write(data + sent, to_send);
      sent += written;
    }
    // If the buffer is full (available == 0), the loop will spin after a delay
    vTaskDelay(1);
  }

  // After the entire message is queued, flush it.
  tud_cdc_write_flush();
}
