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

**Note:** The use of this value as a marker means that if the value `0xFE` or the escape character `0xFD` needs to be sent as part of a command's parameters, they must be escaped. See the **Character Escaping** section below.


### Command Types

Note: `ASCII_SPACE_OFFSET = 0x0F` (15)


1. `TEXT_CMD (0x02)`: Draw a character

Parameters:

* Character to draw
* X position (offset by ASCII_SPACE_OFFSET)
* Y position (offset by ASCII_SPACE_OFFSET)
* Invert flag (0 for normal, 0x7F for inverted)

2. `CLEAR_CMD (0x03)`: Clear screen

Parameters:

* Background color Red (`0x00`-`0xFF`)
* Background color Green (`0x00`-`0xFF`)
* Background color Blue (`0x00`-`0xFF`)

3. `SETCOLOR_CMD (0x04)`: Set foreground color

Parameters:

* Red (`0x00`-`0xFF`, escaped)
* Green (`0x00`-`0xFF`, escaped)
* Blue (`0x00`-`0xFF`, escaped)

4. `SETFONT_CMD (0x05)`

Parameters:

* Font index  (offset by ASCII_SPACE_OFFSET)

Currently the only available fonts are:

 | Index | Font   
 | ----- | -----
 | 0x00  | Hourglass
 | 0x01  | You Squared

5. `DRAWRECT_CMD (0x06)`: Draw a rectangle (can be used for pixel drawing)

Parameters:

* Left (16-bit, Little Endian, escaped)
* Top (16-bit, Little Endian, escaped)
* Width (16-bit, Little Endian, escaped)
* Height (16-bit, Little Endian, escaped)


### Character Escaping

To allow the full range of byte values (`0x00`-`0xFF`) in parameters while reserving `0xFE` as a command marker, an escaping mechanism is used for certain commands (like `SETCOLOR_CMD` and `DRAWRECT_CMD`).

* Escape Character: `0xFD`
* Escape XOR mask: `0x20`

If a parameter byte is either `0xFE` or `0xFD`, it is replaced by a two-byte sequence:
1. The escape character `0xFD`
2. The original byte XORed with `0x20`

For example:
* `0xFE` becomes `0xFD 0xDE`
* `0xFD` becomes `0xFD 0xDD`

### Example Transmission Flow

To give a concrete example, below is a simple example of how a command in the picoTracker firmware is transmitted over USB serial:

```cpp

// Drawing a character 'A' at position (2,3), not inverted
// ASCII_SPACE_OFFSET = 0x0F (15)
remoteUIBuffer[0] = 0xFE;        // Command marker
remoteUIBuffer[1] = TEXT_CMD;    // Draw character command (0x02)
remoteUIBuffer[2] = 'A';         // Character
remoteUIBuffer[3] = 0x11;        // X position (2 + 15 = 17)
remoteUIBuffer[4] = 0x12;        // Y position (3 + 15 = 18)
remoteUIBuffer[5] = 0x00;        // Not inverted
sendToUSBCDC(remoteUIBuffer, 6);
```

### Input Commands

Clients are also able to send input events to the picoTracker. As for output commands, the input commands are prefixed by sending the REMOTE_UI_CMD_MARKER byte.

The events currently supported are:

1. `FULL_REFRESH_CMD (0x02)`: Request sending all the current screen and current font

Parameters:

* None


### Limitations

* Input events other than FULL_REFRESH_CMD are not yet implemented.

### Client Implementation Guidelines

1. Look for REMOTE_UI_CMD_MARKER (`0xFE`)
2. If the next byte is part of a parameter for a command that uses escaping, check if it's the escape character (`0xFD`).
3. Handle potential transmission errors
4. Implement appropriate rendering based on received commands