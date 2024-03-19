#ifndef PICO_REMOTE_UI_H_
#define PICO_REMOTE_UI_H_

#ifdef USB_REMOTE_UI

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
// Because other diagnostic messages may also be printed to the USB console
// output remote UI clients should pay careful attention to using the
// REMOTE_UI_CMD_MARKER and also verify the expected byte count length of each
// command received.
//
// SetPalette is currently not implemented, so remote clients just need to use
// their own color palette for now.

enum RemoteUICommand {
  DRAW_CMD = 0x32,
  CLEAR_CMD = 0x33,
  SETCOLOR_CMD = 0x34,
  SETPALETTE_CMD = 0x04
};

#define REMOTE_UI_CMD_MARKER 0xFD
#define UART_ASCII_OFFSET 32

#endif
#endif