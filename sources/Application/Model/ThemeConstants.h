#ifndef _THEME_CONSTANTS_H_
#define _THEME_CONSTANTS_H_

// Define default color values to be used across the application
namespace ThemeConstants {
// Color constants
const uint32_t DEFAULT_BACKGROUND = 0x0F0F0F;
const uint32_t DEFAULT_FOREGROUND = 0xADADAD;
const uint32_t DEFAULT_HICOLOR1 = 0x846F94;
const uint32_t DEFAULT_HICOLOR2 = 0x6B316B;
const uint32_t DEFAULT_CONSOLECOLOR = 0xFF00FF;
const uint32_t DEFAULT_CURSORCOLOR = 0x776B56;
const uint32_t DEFAULT_INFOCOLOR = 0x29EE3D;
const uint32_t DEFAULT_WARNCOLOR = 0xEFFA52;
const uint32_t DEFAULT_ERRORCOLOR = 0xE84D15;
const uint32_t DEFAULT_ACCENT = 0x00FF00;
const uint32_t DEFAULT_ACCENT_ALT = 0xFF0000;
const uint32_t DEFAULT_EMPHASIS = 0xFFA500;
// const uint32_t DEFAULT_RESERVED1 = 0x0000FF;
// const uint32_t DEFAULT_RESERVED2 = 0x555555;
// const uint32_t DEFAULT_RESERVED3 = 0x777777;
// const uint32_t DEFAULT_RESERVED4 = 0xFFFF00;

// Font constants
const int DEFAULT_UIFONT = 0x0;
// Default theme name - using inline to avoid multiple definition errors
inline const char *DEFAULT_THEME_NAME = "Default";
} // namespace ThemeConstants

#endif // _THEME_CONSTANTS_H_
