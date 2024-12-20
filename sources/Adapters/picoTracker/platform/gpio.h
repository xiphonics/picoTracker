#ifndef _PLATFORM_PICO_GPIO_H_
#define _PLATFORM_PICO_GPIO_H_

// POWER
#if PICO_RP2350
#define POWER_BTN 0
#define CHARGER_PSEL 1
#define CHARGER_CE 2
#define CHARGER_OTG 3
#define CHARGER_INT 4
#define FUEL_GPOUT 5
#else
#define BATT_VOLTAGE_IN 29
#endif

// I2C
#if PICO_RP2350
#define I2C_SDA 6
#define I2C_SCL 7
#endif

// Display
#define DISPLAY_SPI spi1
#if PICO_RP2350
#define DISPLAY_CS 13
#define DISPLAY_DC 9
#define DISPLAY_RESET 10
#define DISPLAY_SCK 14
#define DISPLAY_MOSI 11
#define DISPLAY_MISO 12
#define DISPLAY_PWM 8
#else
#define DISPLAY_CS 20
#define DISPLAY_DC 21
#define DISPLAY_RESET 22
#define DISPLAY_SCK 26
#define DISPLAY_MOSI 27
#define DISPLAY_MISO 28
#define DISPLAY_PWM 23
#endif

// MIDI
#define MIDI_BAUD_RATE 31250
#if PICO_RP2350
#define MIDI_UART uart1
#define MIDI_OUT_PIN 42
#define MIDI_IN_PIN 43
#else
#define MIDI_UART uart0
#define MIDI_OUT_PIN 0
#define MIDI_IN_PIN 1
#endif

#define DEBUG_UART uart1

// SD Card
/* SPI (SPI1) */
// #define SD_SPI spi0
// #define SD_SPI_SCK 2
// #define SD_SPI_MOSI 3
// #define SD_SPI_MISO 4
// #define SD_SPI_CS 7

/* SDIO */
#define SDIO_PIO pio1
#define SDIO_CMD_SM 0
#define SDIO_DATA_SM 1
#define SDIO_DMA_CH 4
#define SDIO_DMA_CHB 5
#if PICO_RP2350
#define SDIO_CLK 21
#define SDIO_CMD 22
#define SDIO_D0 23
#define SDIO_D1 24
#define SDIO_D2 25
#define SDIO_D3 26
#define SD_DET 27
#else
#define SDIO_CLK 2
#define SDIO_CMD 3
#define SDIO_D0 4
#define SDIO_D1 5
#define SDIO_D2 6
#define SDIO_D3 7
#endif

// Input
#if PICO_RP2350
#define INPUT_LEFT 32
#define INPUT_DOWN 33
#define INPUT_RIGHT 34
#define INPUT_UP 35
#define INPUT_LT 36
#define INPUT_B 37
#define INPUT_A 38
#define INPUT_RT 39
#define INPUT_PLAY 40
#define INPUT_THUMB 41
#define INPUT_HORIZ 45
#define INPUT_VERT 46
#else
#define INPUT_LEFT 8
#define INPUT_DOWN 9
#define INPUT_RIGHT 10
#define INPUT_UP 11
#define INPUT_LT 12
#define INPUT_B 13
#define INPUT_A 14
#define INPUT_RT 15
#define INPUT_PLAY 16
#endif

// Sound
#define AUDIO_PIO pio0
#define AUDIO_SM 0
#define AUDIO_DMA 0
#define AUDIO_DMA_IRQ 0
#if PICO_RP2350
// TODO: we'll need more stuff to support CODEC
#define AUDIO_MCLK_SM 1
#define CODEC_RESET 15
#define AUDIO_SDATA 17
#define CODEC_DIN 16
#define AUDIO_BCLK 18 // BCLK and LRCLK HAVE to be consecutive
#define AUDIO_LRCLK 19
#define CODEC_MCLK 20
#else
#define AUDIO_SDATA 17
#define AUDIO_BCLK 18 // BCLK and LRCLK HAVE to be consecutive
#define AUDIO_LRCLK 19
#endif

#endif
