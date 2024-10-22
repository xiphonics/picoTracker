#include <cstdint>
#ifndef PICO_REMOTE_UI_H_
#define PICO_REMOTE_UI_H_

#include "tusb.h"

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
// been attempted to be offset by 32 to push them into the printable char range.
//
// Remote UI clients should pay careful attention to using the
// REMOTE_UI_CMD_MARKER and also verify the expected byte count length of each
// command received to check for any transmission errors.
//
// SetPalette is currently not implemented, so remote clients just need to use
// their own color palette for now.

#define ITF_NUM_CDC_0 0

#define USB_TIMEOUT_US 500000

static void sendToUSBCDC(char buf[], int length) {
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

enum RemoteUICommand {
  DRAW_CMD = 0x32,
  CLEAR_CMD = 0x33,
  SETCOLOR_CMD = 0x34,
  SETPALETTE_CMD = 0x04
};

#define REMOTE_UI_CMD_MARKER 0xFD
#define UART_ASCII_OFFSET 32

#endif