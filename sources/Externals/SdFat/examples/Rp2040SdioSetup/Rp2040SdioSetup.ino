// RP2040 PIO SDIO setup and test.
/*
This example requires a SDIO Card socket with the following six lines.

CLK - A clock signal sent to the card by the MCU.
CMD - A bidirectional line for for commands and responses.
DAT[0:3] - Four bidirectional lines for data transfer.

CLK and CMD can be connected to any GPIO pins. DAT[0:3] can be connected 
to any four consecutive GPIO pins in the order DAT0, DAT1, DAT2, DAT3.

For testing, I use two RP2040 boards.  The first is an 
Adafruit Metro RP2040 which has a builtin SDIO socket.

https://learn.adafruit.com/adafruit-metro-rp2040

The second is a Raspberry Pi Pico with this breakout board.

https://learn.adafruit.com/adafruit-microsd-spi-sdio

Wires should be short since signals can be as fast as 63 MHz. 
*/
#define DISABLE_FS_H_WARNING  // Disable warning for type File not defined.
#include "Rp2040Variants.h"
#include "SdFat.h"
//------------------------------------------------------------------------------
// Pin definitions. Edit for your setup. 
// See the Rp2040Variants.h file with this example for variant symbols.
//
#if defined(ARDUINO_ADAFRUIT_METRO_RP2040)
#define CLK_PIN 18
#define CMD_PIN 19
#define DAT0_PIN 20 // DAT1: pin 21, DAT2: pin 22, DAT3: pin 23.
#endif  // defined(ARDUINO_ADAFRUIT_METRO_RP2040)

// This is my setup for debug with Pi Pico.
// CLK_PIN, CMD_PIN and DAT0_PIN can be any GPIO pins. 
#if defined(ARDUINO_RASPBERRY_PI_PICO)
#define CLK_PIN 16  
#define CMD_PIN 17
#define DAT0_PIN 18 // DAT1: pin 19, DAT2: pin 20, DAT3: pin 21.
#endif  // defined(ARDUINO_RASPBERRY_PI_PICO)

#if defined(CLK_PIN) && defined(CMD_PIN) && defined(DAT0_PIN)
#define SD_CONFIG SdioConfig(CLK_PIN, CMD_PIN, DAT0_PIN)
#else  // defined(CLK_PIN) && defined(CMD_PIN) && defined(DAT0_PIN)
#warning "Undefined SD_CONFIG"
#endif // defined(CLK_PIN) && defined(CMD_PIN) && defined(DAT0_PIN)

//------------------------------------------------------------------------------
// Class File is not defined by SdFat since the RP2040 system defines it.
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 3

#if SD_FAT_TYPE == 1
SdFat32 sd;
File32 file;
#elif SD_FAT_TYPE == 2
SdExFat sd;
ExFile file;
#elif SD_FAT_TYPE == 3
SdFs sd;
FsFile file;
#else  // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif  // SD_FAT_TYPE

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    yield();   
  }
  Serial.println("Type any character to start\n");
  while (!Serial.available()) {
    yield();
  }
  Serial.print("BOARD_NAME: ");
  Serial.println(BOARD_NAME);
  Serial.print("Variant Symbol: ");
  Serial.println(variantSymbol());
  Serial.println();
#if defined(SD_CONFIG)
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
  }
  Serial.println("Card successfully initialized.");
  Serial.println("\nls:");
  sd.ls(LS_A | LS_DATE | LS_SIZE);  // Add LS_R for recursive list.
  Serial.println("\nDone! Try the bench example next.");
#else  // #if defined(SD_CONFIG)
  Serial.println("Error: SD_CONFIG undefined.");
#endif
}

void loop() {

}
