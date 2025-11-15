---
title: picoTracker Remote User Interface Protocol
template: page
---

## Overview

The Remote UI protocol is a communication mechanism that allows rendering the picoTracker's user interface over a USB serial connection. It enables a remote client to:

* Receive UI rendering commands
* (NOT YET IMPLEMENTED) Send button input events back to the device

## Command Structure

### Command Marker

Every command starts with a fixed marker: `0xFE` (REMOTE_UI_CMD_MARKER). This allows clients to verify the start of a valid command.

**Note:** That the use of this value as a marker to start commands means that the characters 0xFE and 0xFF from the extended ASCII range are not allowed in the protocols command parameter values.


### Command Types

Note: `ASCII_SPACE_OFFSET = 0xF`


1. TEXT_CMD (0x2): Draw a character

Parameters:

* Character to draw
* X position (offset by ASCII_SPACE_OFFSET)
* Y position (offset by ASCII_SPACE_OFFSET)
* Invert flag (0 for normal, 0x7F for inverted)

2. `CLEAR_CMD (0x3)`: Clear screen

Parameters:

* Background color in RGB888 format

3. SETCOLOR_CMD (0x4): Set foreground color

Parameters:

* Color in RGB888 format

4. SETFONT_CMD (0x5)

Parameters:

* Font index  (offset by ASCII_SPACE_OFFSET)

Currently the only available fonts are:

 | Index | Font   
 | ----- | -----
 | 0     | Hourglass
 | 1     | You Squared



### Example Transmission Flow

To give a concrete example, below is a simple example of how a command in the picoTracker firmware istransmitted over USB serial:

```cpp

// Drawing a character 'A' at position (2,3), not inverted
remoteUIBuffer[0] = 0xFE;        // Command marker
remoteUIBuffer[1] = DRAW_CMD;    // Draw command
remoteUIBuffer[2] = 'A';         // Character
remoteUIBuffer[3] = 34;          // X position (2 + 32)
remoteUIBuffer[4] = 35;          // Y position (3 + 32)
remoteUIBuffer[5] = 32;          // Not inverted
sendToUSBCDC(remoteUIBuffer, 6);
```

### Input Commands

Clients are also able to send input events to the picoTracker. As for output commands, the input commands are prefixed by sending the REMOTE_UI_CMD_MARKER byte.

The events currently supported are:

1. FULL_REFRESH_CMD (0x2): Request sending all the current screen and current font

Parameters:

* None


### Limitations

* Input events other than FULL_REFRESH_CMD are not yet implemented.

### Client Implementation Guidelines

1. Look for REMOTE_UI_CMD_MARKER (0xFD)
1. Verify command type
1. Subtract 32 from positional/color values
1. Handle potential transmission errors
1. Implement appropriate rendering based on received commands