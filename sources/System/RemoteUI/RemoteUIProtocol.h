/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _REMOTEUIPROTOCOL_H_
#define _REMOTEUIPROTOCOL_H_

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

#include "UIFramework/BasicDatas/GUIEvent.h"

#define ASCII_SPACE_OFFSET 0xF
#define INVERT_ON 0x7F
#define REMOTE_UI_ESC_CHAR 0xFD
#define REMOTE_UI_ESC_XOR 0x20

enum RemoteUICommand {
  REMOTE_UI_CMD_MARKER = 0xFE,
  TEXT_CMD = 0x02,
  CLEAR_CMD = 0x03,
  SETCOLOR_CMD = 0x04,
  SETFONT_CMD = 0x05,
  DRAWRECT_CMD = 0x06
};

enum RemoteInputCommand {
  REMOTE_INPUT_CMD_MARKER = 0xFE,
  FULL_REFRESH_CMD = 0x02,
};

// classic picotracker mapping
static GUIEventPadButtonType eventMappingPico[10] = {
    EPBT_LEFT,  // SW1
    EPBT_DOWN,  // SW2
    EPBT_RIGHT, // SW3
    EPBT_UP,    // SW4
    EPBT_L,     // SW5
    EPBT_B,     // SW6
    EPBT_A,     // SW7
    EPBT_R,     // SW8
    EPBT_START, // SW9
    EPBT_SELECT // No SW
};

#define to_rgb565(color)                                                       \
  ((color._r & 0b11111000) << 8) | ((color._g & 0b11111100) << 3) |            \
      (color._b >> 3)

#endif