/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "advRemoteUI.h"
#include <System/Console/Trace.h>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "stm32h7xx_hal.h" // Change to your specific STM32 series HAL header
// #include "usbd_cdc_if.h"   // Header for the USB CDC Interface

// Define a timeout in milliseconds to prevent getting stuck
#define USB_TX_TIMEOUT_MS 50

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

void sendToUSBCDC(char buf[], int length) {
  // The STM32 middleware function requires a uint8_t pointer, so we cast the
  // buffer.

  // Get the current time using the HAL tick (which is in milliseconds).
  uint32_t startTime = HAL_GetTick();

  // CDC_Transmit_FS returns USBD_BUSY if the previous transmission is not
  // complete. We loop until it returns USBD_OK, indicating the data has been
  // accepted.
  while (CDC_Transmit_HS((uint8_t *)buf, strlen(buf)) == USBD_BUSY)
    ;
}
