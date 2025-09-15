// Just the RemoteUIProtocol header for now
#include "RemoteUIProtocol.h"

// Helper function for byte escaping.
static uint16_t addByteEscaped(char *buffer, uint16_t bufferIndex, char byte) {
  if (byte == REMOTE_UI_CMD_MARKER || byte == REMOTE_UI_ESC_CHAR) {
    buffer[bufferIndex++] = REMOTE_UI_ESC_CHAR;
    buffer[bufferIndex++] = byte ^ REMOTE_UI_ESC_XOR;
  } else {
    buffer[bufferIndex++] = byte;
  }
  return bufferIndex;
}

// Helper function to add a 16-bit value, escaping each of its two bytes.
static uint16_t add16bitEscaped(char *buffer, uint16_t bufferIndex,
                                uint16_t val) {
  bufferIndex = addByteEscaped(buffer, bufferIndex, val & 0xFF); // Add LSB
  bufferIndex =
      addByteEscaped(buffer, bufferIndex, (val >> 8) & 0xFF); // Add MSB
  return bufferIndex;
}

void remoteUIFontCommand(uint8_t uifontIndex, char *buffer) {
  buffer[0] = REMOTE_UI_CMD_MARKER;
  buffer[1] = SETFONT_CMD;
  buffer[2] = uifontIndex + ASCII_SPACE_OFFSET;
}

void remoteUIDrawCharCommand(const char c, uint8_t x, uint8_t y, bool invert,
                             char *buffer) {
  buffer[0] = REMOTE_UI_CMD_MARKER;
  buffer[1] = TEXT_CMD;
  buffer[2] = c;
  buffer[3] = x + ASCII_SPACE_OFFSET; // to avoid sending NUL (aka 0)
  buffer[4] = y + ASCII_SPACE_OFFSET;
  buffer[5] = invert ? 127 : 0;
}

uint16_t remoteUIDrawRectCommand(int left, int top, int width, int height,
                                 char *buffer) {
  uint16_t bufferIndex = 0;
  buffer[bufferIndex++] = REMOTE_UI_CMD_MARKER;
  buffer[bufferIndex++] = DRAWRECT_CMD;
  bufferIndex = add16bitEscaped(buffer, bufferIndex, left);
  bufferIndex = add16bitEscaped(buffer, bufferIndex, top);
  bufferIndex = add16bitEscaped(buffer, bufferIndex, width);
  bufferIndex = add16bitEscaped(buffer, bufferIndex, height);
  return bufferIndex;
}

void remoteUIClearCommand(unsigned short r, unsigned short g, unsigned short b,
                          char *buffer) {
  buffer[0] = REMOTE_UI_CMD_MARKER;
  buffer[1] = CLEAR_CMD;
  buffer[2] = r;
  buffer[3] = g;
  buffer[4] = b;
}

uint16_t remoteUISetColorCommand(unsigned short r, unsigned short g,
                                 unsigned short b, char *buffer) {
  uint16_t bufferIndex = 0;
  buffer[bufferIndex++] = REMOTE_UI_CMD_MARKER;
  buffer[bufferIndex++] = SETCOLOR_CMD;
  bufferIndex = addByteEscaped(buffer, bufferIndex, r);
  bufferIndex = addByteEscaped(buffer, bufferIndex, g);
  bufferIndex = addByteEscaped(buffer, bufferIndex, b);
  return bufferIndex;
}
