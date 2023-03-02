#include "ili9341.h"
#include "pico/stdlib.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*

 (pin 1) VCC        5V/3.3V power input
 (pin 2) GND        Ground
 (pin 3) CS         LCD chip select signal, low level enable
 (pin 4) RESET      LCD reset signal, low level reset
 (pin 5) DC/RS      LCD register / data selection signal; high level: register,
 low level: data (pin 6) SDI(MOSI)  SPI bus write data signal (pin 7) SCK SPI
 bus clock signal (pin 8) LED        Backlight control; if not controlled,
 connect 3.3V always bright (pin 9) SDO(MISO)  SPI bus read data signal;
 optional

 */

ili9341_config_t ili9341_config = {.port = spi0,
                                   .pin_miso = 4,
                                   .pin_cs = 5,
                                   .pin_sck = 6,
                                   .pin_mosi = 7,
                                   .pin_reset = 2,
                                   .pin_dc = 3};

static inline void cs_select() {
  asm volatile("nop \n nop \n nop");
  gpio_put(ili9341_config.pin_cs, 0); // Active low
  asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect() {
  asm volatile("nop \n nop \n nop");
  gpio_put(ili9341_config.pin_cs, 1);
  asm volatile("nop \n nop \n nop");
}

void ili9341_set_command(uint8_t cmd) {
  cs_select();
  gpio_put(ili9341_config.pin_dc, 0);
  spi_write_blocking(ili9341_config.port, &cmd, 1);
  gpio_put(ili9341_config.pin_dc, 1);
  cs_deselect();
}

void ili9341_command_param16(uint16_t data) {
  ili9341_command_param(data >> 8);
  ili9341_command_param(data & 0xFF);
}

void ili9341_command_param(uint8_t data) {
  cs_select();
  spi_write_blocking(ili9341_config.port, &data, 1);
  cs_deselect();
}

inline void ili9341_start_writing() { cs_select(); }

void ili9341_write_data(void *buffer, int bytes) {
  cs_select();
  spi_write_blocking(ili9341_config.port, buffer, bytes);
  cs_deselect();
}

void ili9341_write_data_continuous(void *buffer, int bytes) {
  spi_write_blocking(ili9341_config.port, buffer, bytes);
}

inline void ili9341_stop_writing() { cs_deselect(); }

void ili9341_init() {
  // This example will use SPI0 at 0.5MHz.
  spi_init(ili9341_config.port, 500 * 1000);
  int baudrate = spi_set_baudrate(ili9341_config.port, 75000 * 1000);
  printf("Display SPI Baudrate: %i\n", baudrate);

  gpio_set_function(ili9341_config.pin_miso, GPIO_FUNC_SPI);
  gpio_set_function(ili9341_config.pin_sck, GPIO_FUNC_SPI);
  gpio_set_function(ili9341_config.pin_mosi, GPIO_FUNC_SPI);

  // Chip select is active-low, so we'll initialise it to a driven-high state
  gpio_init(ili9341_config.pin_cs);
  gpio_set_dir(ili9341_config.pin_cs, GPIO_OUT);
  gpio_put(ili9341_config.pin_cs, 0);

  // Reset is active-low
  gpio_init(ili9341_config.pin_reset);
  gpio_set_dir(ili9341_config.pin_reset, GPIO_OUT);
  gpio_put(ili9341_config.pin_reset, 1);

  // high = command, low = data
  gpio_init(ili9341_config.pin_dc);
  gpio_set_dir(ili9341_config.pin_dc, GPIO_OUT);
  gpio_put(ili9341_config.pin_dc, 0);

  sleep_ms(10);
  gpio_put(ili9341_config.pin_reset, 0);
  sleep_ms(10);
  gpio_put(ili9341_config.pin_reset, 1);

  ili9341_set_command(0x01); // soft reset
  sleep_ms(100);

  ili9341_set_command(ILI9341_GAMMASET);
  ili9341_command_param(0x01);

  // positive gamma correction
  ili9341_set_command(ILI9341_GMCTRP1);
  ili9341_write_data((uint8_t[15]){0x0f, 0x31, 0x2b, 0x0c, 0x0e, 0x08, 0x4e,
                                   0xf1, 0x37, 0x07, 0x10, 0x03, 0x0e, 0x09,
                                   0x00},
                     15);

  // negative gamma correction
  ili9341_set_command(ILI9341_GMCTRN1);
  ili9341_write_data((uint8_t[15]){0x00, 0x0e, 0x14, 0x03, 0x11, 0x07, 0x31,
                                   0xc1, 0x48, 0x08, 0x0f, 0x0c, 0x31, 0x36,
                                   0x0f},
                     15);

  // memory access control
  ili9341_set_command(ILI9341_MADCTL);
  ili9341_command_param(0x48);

  // pixel format
  ili9341_set_command(ILI9341_PIXFMT);
  ili9341_command_param(0x55); // 16-bit

  // frame rate; default, 70 Hz
  ili9341_set_command(ILI9341_FRMCTR1);
  ili9341_command_param(0x00);
  ili9341_command_param(0x1B);

  // exit sleep
  ili9341_set_command(ILI9341_SLPOUT);

  // display on
  ili9341_set_command(ILI9341_DISPON);

  //

  // column address set
  ili9341_set_command(ILI9341_CASET);
  ili9341_command_param(0x00);
  ili9341_command_param(0x00); // start column
  ili9341_command_param(0x00);
  ili9341_command_param(0xef); // end column -> 239

  // page address set
  ili9341_set_command(ILI9341_PASET);
  ili9341_command_param(0x00);
  ili9341_command_param(0x00); // start page
  ili9341_command_param(0x01);
  ili9341_command_param(0x3f); // end page -> 319

  ili9341_set_command(ILI9341_RAMWR);
}

uint16_t swap_bytes(uint16_t color) { return (color >> 8) | (color << 8); }
