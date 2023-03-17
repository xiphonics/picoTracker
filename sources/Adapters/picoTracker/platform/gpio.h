#ifndef _PLATFORM_PICO_GPIO_H_
#define _PLATFORM_PICO_GPIO_H_

// Debug
#define DEBUG_UART uart0
#define DEBUG_BAUD_RATE
#define DEBUG_TX 0
#define DEBUG_RX 1

// Display (SPI0)
#define DISPLAY_SPI   spi0
#define DISPLAY_RESET 2
#define DISPLAY_DC    3
#define DISPLAY_MISO  4
#define DISPLAY_CS    5
#define DISPLAY_SCK   6
#define DISPLAY_MOSI  7

// Midi (UART1)
#define MIDI_UART      uart1
#define MIDI_BAUD_RATE 31250
#define MIDI_OUT_PIN   8
#define MIDI_IN_PIN    9

// SD Card
/* SPI (SPI1) */
#define SD_SPI      spi1
#define SD_SPI_SCK  10
#define SD_SPI_MOSI 11
#define SD_SPI_MISO 12
#define SD_SPI_CS   15

/* SDIO */
#define SDIO_PIO     pio1
#define SDIO_CMD_SM  0
#define SDIO_DATA_SM 1
#define SDIO_DMA_CH  4
#define SDIO_DMA_CHB 5
#define SDIO_CLK     10
#define SDIO_CMD     11
#define SDIO_D0      12
#define SDIO_D1      13
#define SDIO_D2      14
#define SDIO_D3      15

// Input
#define INPUT_COL1 16
#define INPUT_COL2 17
#define INPUT_COL3 18
#define INPUT_ROW1 19
#define INPUT_ROW2 20
#define INPUT_ROW3 21

// Sound
// TODO: Check if this is necessary for i2s
#define PIN_DCDC_PSM_CTRL 23

#define AUDIO_PIO     pio0
#define AUDIO_SM      0
#define AUDIO_DMA     0
#define AUDIO_DMA_IRQ 0
#define AUDIO_MCLK
#define AUDIO_BCLK    26 // BCLK and LRCLK HAVE to be consecutive
#define AUDIO_LRCLK   27
#define AUDIO_SDATA   28

#endif
