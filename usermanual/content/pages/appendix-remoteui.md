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

Every command starts with a fixed marker: 0xFD (REMOTE_UI_CMD_MARKER)

This helps clients verify the start of a valid command.

### Command Types

1. DRAW_CMD (0x32): Draw a character

Parameters:

* Character to draw
* X position (offset by 32)
* Y position (offset by 32)
* Invert flag (32 for normal, 127 for inverted)

2. CLEAR_CMD (0x33): Clear screen

Parameters:

* Background color (offset by 32)

3. SETCOLOR_CMD (0x34): Set foreground color

Parameters:

* Color index (offset by 32)

4. SETPALETTE_CMD (0x04): Not currently implemented

Remote clients should use their own color palette

### Example Transmission Flow

To give a concrete example, below is a simple example of how a command in the picoTracker firmware istransmitted over USB serial:

```cpp

// Drawing a character 'A' at position (2,3), not inverted
remoteUIBuffer[0] = 0xFD;        // Command marker
remoteUIBuffer[1] = DRAW_CMD;    // Draw command
remoteUIBuffer[2] = 'A';         // Character
remoteUIBuffer[3] = 34;          // X position (2 + 32)
remoteUIBuffer[4] = 35;          // Y position (3 + 32)
remoteUIBuffer[5] = 32;          // Not inverted
sendToUSBCDC(remoteUIBuffer, 6);
```

### Limitations

SetPalette command is not implemented
SetFont command is not implemented

### Client Implementation Guidelines

1. Look for REMOTE_UI_CMD_MARKER (0xFD)
1. Verify command type
1. Subtract 32 from positional/color values
1. Handle potential transmission errors
1. Implement appropriate rendering based on received commands