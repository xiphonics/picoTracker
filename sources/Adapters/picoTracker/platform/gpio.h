#ifndef _PLATFORM_PICO_GPIO_H_
#define _PLATFORM_PICO_GPIO_H_

// Display (SPI0)
#define DISPLAY_SPI   spi1
#define DISPLAY_CS    21
#define DISPLAY_RESET 22
#define DISPLAY_SCK   26
#define DISPLAY_MOSI  27
#define DISPLAY_DC    28

// Midi (UART1)
#define MIDI_UART      uart0
#define MIDI_BAUD_RATE 31250
#define MIDI_OUT_PIN   0
#define MIDI_IN_PIN    1

// SD Card
/* SPI (SPI1) */
#define SD_SPI      spi0
#define SD_SPI_SCK  2
#define SD_SPI_MOSI 3
#define SD_SPI_MISO 4
#define SD_SPI_CS   7

/* SDIO */
#define SDIO_PIO     pio1
#define SDIO_CMD_SM  0
#define SDIO_DATA_SM 1
#define SDIO_DMA_CH  4
#define SDIO_DMA_CHB 5
#define SDIO_CLK     2
#define SDIO_CMD     3
#define SDIO_D0      4
#define SDIO_D1      5
#define SDIO_D2      6
#define SDIO_D3      7

// Input
#define INPUT_LEFT  8
#define INPUT_DOWN  9
#define INPUT_RIGHT 10
#define INPUT_UP    11
#define INPUT_LT    12
#define INPUT_B     13
#define INPUT_A     14
#define INPUT_RT    15
#define INPUT_PLAY  16

// Sound
// TODO: Check if this is necessary for i2s
#define PIN_DCDC_PSM_CTRL 23

#define AUDIO_PIO     pio0
#define AUDIO_SM      0
#define AUDIO_DMA     0
#define AUDIO_DMA_IRQ 0
#define AUDIO_MCLK    17
#define AUDIO_BCLK    18 // BCLK and LRCLK HAVE to be consecutive
#define AUDIO_LRCLK   19
#define AUDIO_SDATA   20

#endif
