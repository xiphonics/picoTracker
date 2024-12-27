#include <cstdint>
#ifndef PICO_REMOTE_UI_H_
#define PICO_REMOTE_UI_H_

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

#define ITF_NUM_CDC_0 0
#define USB_TIMEOUT_US 500000
#define ASCII_SPACE_OFFSET 0xF
#define INVERT_ON 0x7F

enum RemoteUICommand {
  REMOTE_UI_CMD_MARKER = 0xFE,
  TEXT_CMD = 0x02,
  CLEAR_CMD = 0x03,
  SETCOLOR_CMD = 0x04,
  SETFONT_CMD = 0x05
};

enum RemoteInputCommand {
  REMOTE_INPUT_CMD_MARKER = 0xFE,
  FULL_REFRESH_CMD = 0x02,
};

void sendToUSBCDC(char buf[], int length);

#endif