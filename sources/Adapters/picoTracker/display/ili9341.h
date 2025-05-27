#ifndef _ILI9341_H
#define _ILI9341_H

#include "hardware/spi.h"
#include "pico/stdlib.h"
#include <stdint.h>

#define ILI9341_TFTWIDTH 240  // ILI9341 max TFT width
#define ILI9341_TFTHEIGHT 320 // ILI9341 max TFT height

#define ILI9341_NOP 0x00     // No-op register
#define ILI9341_SWRESET 0x01 // Software reset register
#define ILI9341_RDDID 0x04   // Read display identification information
#define ILI9341_RDDST 0x09   // Read Display Status

#define ILI9341_SLPIN 0x10  // Enter Sleep Mode
#define ILI9341_SLPOUT 0x11 // Sleep Out
#define ILI9341_PTLON 0x12  // Partial Mode ON
#define ILI9341_NORON 0x13  // Normal Display Mode ON

#define ILI9341_RDMODE 0x0A     // Read Display Power Mode
#define ILI9341_RDMADCTL 0x0B   // Read Display MADCTL
#define ILI9341_RDPIXFMT 0x0C   // Read Display Pixel Format
#define ILI9341_RDIMGFMT 0x0D   // Read Display Image Format
#define ILI9341_RDSELFDIAG 0x0F // Read Display Self-Diagnostic Result

#define ILI9341_INVOFF 0x20   // Display Inversion OFF
#define ILI9341_INVON 0x21    // Display Inversion ON
#define ILI9341_GAMMASET 0x26 // Gamma Set
#define ILI9341_DISPOFF 0x28  // Display OFF
#define ILI9341_DISPON 0x29   // Display ON

#define ILI9341_CASET 0x2A // Column Address Set
#define ILI9341_PASET 0x2B // Page Address Set
#define ILI9341_RAMWR 0x2C // Memory Write
#define ILI9341_RAMRD 0x2E // Memory Read

#define ILI9341_PTLAR 0x30    // Partial Area
#define ILI9341_VSCRDEF 0x33  // Vertical Scrolling Definition
#define ILI9341_MADCTL 0x36   // Memory Access Control
#define ILI9341_VSCRSADD 0x37 // Vertical Scrolling Start Address
#define ILI9341_PIXFMT 0x3A   // COLMOD: Pixel Format Set

#define ILI9341_FRMCTR1 0xB1 // Frame Rate Control (In Normal Mode/Full Colors)
#define ILI9341_FRMCTR2 0xB2 // Frame Rate Control (In Idle Mode/8 colors)
#define ILI9341_FRMCTR3 0xB3 // Frame Rate control (In Partial Mode/Full Colors)
#define ILI9341_INVCTR 0xB4  // Display Inversion Control
#define ILI9341_DFUNCTR 0xB6 // Display Function Control

#define ILI9341_PWCTR1 0xC0 // Power Control 1
#define ILI9341_PWCTR2 0xC1 // Power Control 2
#define ILI9341_PWCTR3 0xC2 // Power Control 3
#define ILI9341_PWCTR4 0xC3 // Power Control 4
#define ILI9341_PWCTR5 0xC4 // Power Control 5
#define ILI9341_PWCTR6 0xFC // Power Control 6

#define ILI9341_VMCTR1 0xC5 // VCOM Control 1
#define ILI9341_VMCTR2 0xC7 // VCOM Control 2

#define ILI9341_RDID1 0xDA // Read ID 1
#define ILI9341_RDID2 0xDB // Read ID 2
#define ILI9341_RDID3 0xDC // Read ID 3
#define ILI9341_RDID4 0xDD // Read ID 4

#define ILI9341_GMCTRP1 0xE0 // Positive Gamma Correction
#define ILI9341_GMCTRN1 0xE1 // Negative Gamma Correction

// Allowable frame rate codes for ILI9341_FRMCTR1 (Identifier is in Hz)
#define FRAMERATE_111 0x01
#define FRAMERATE_105 0x02
#define FRAMERATE_99 0x03
#define FRAMERATE_94 0x04
#define FRAMERATE_90 0x05
#define FRAMERATE_86 0x06
#define FRAMERATE_82 0x07
#define FRAMERATE_78 0x08
#define FRAMERATE_75 0x09
#define FRAMERATE_72 0x0A
#define FRAMERATE_69 0x0B
#define FRAMERATE_67 0x0C
#define FRAMERATE_64 0x0D
#define FRAMERATE_62 0x0E
#define FRAMERATE_60 0x0F // 60 is default
#define FRAMERATE_58 0x10
#define FRAMERATE_57 0x11
#define FRAMERATE_55 0x12
#define FRAMERATE_53 0x13
#define FRAMERATE_52 0x14
#define FRAMERATE_50 0x15
#define FRAMERATE_49 0x16
#define FRAMERATE_48 0x17
#define FRAMERATE_46 0x18
#define FRAMERATE_45 0x19
#define FRAMERATE_44 0x1A
#define FRAMERATE_43 0x1B
#define FRAMERATE_42 0x1C
#define FRAMERATE_41 0x1D
#define FRAMERATE_40 0x1E
#define FRAMERATE_39 0x1F

#define MAX_VSYNC_SCANLINES 254

extern const uint8_t font6x8[];

void ili9341_init();
void ili9341_set_command(uint8_t cmd);
void ili9341_command_param(uint8_t data);
void ili9341_command_param16(uint16_t data);
void ili9341_write_data(void *buffer, int bytes);
void ili9341_start_writing();
void ili9341_stop_writing();
void ili9341_write_data_continuous(void *biffer, int bytes);
#endif
