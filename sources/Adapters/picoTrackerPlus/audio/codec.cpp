#include "Adapters/picoTrackerPlus/platform/gpio.h"
#include "hardware/gpio.h"
#include "i2c_reg_io.h"
#include "mclk.pio.h"
#include <stdint.h>
#include <stdio.h>

void setupCodecForDAC(i2c_inst_t *i2c, uint8_t address) {

  // Select Page 0
  reg_write_byte(i2c, address, 0x0, 0x0);
  // Initialize the device through software reset
  reg_write_byte(i2c, address, 0x1, 0x1);

  // pause while codec resets, takes 1ms so be safe and wait 2ms
  sleep_ms(2);

  // setup lock dividers
  // NDAC divider set to 1, bit D7 set to 1 to power up NDAC
  reg_write_byte(i2c, address, 0x0B, 0x81);

  // MDAC divider set to 2, bit D7 set to 1 to power up MDAC
  reg_write_byte(i2c, address, 0x0C, 0x82);

  // DOSR divider set to 128
  // Note high 2 bits of DOSR in reg 0x0D, low byte in reg 0x0E
  reg_write_byte(i2c, address, 0x0D, 0x0);
  reg_write_byte(i2c, address, 0x0E, 0x30);

  // now that clock dividers are setup, configure the power supplies

  // select Page 1
  reg_write_byte(i2c, address, 0x0, 0x01);

  // Power up AVDD LDO
  reg_write_byte(i2c, address, 0x2, 0x09);
  // # Disable weak AVDD in presence of external
  // # AVDD supply
  reg_write_byte(i2c, address, 0x1, 0x08);
  // # Enable Master Analog Power Control
  // # Power up AVDD LDO
  reg_write_byte(i2c, address, 0x2, 0x01);

  // # Set full chip common mode to 0.9V
  // # HP output CM = 1.65V
  // # HP driver supply = LDOin voltage
  // # Line output CM = 1.65V
  // # Line output supply = LDOin voltage
  reg_write_byte(i2c, address, 0x0A, 0x3B);

  // # Select DAC PTM_P3/4
  reg_write_byte(i2c, address, 0x3, 0x00);
  reg_write_byte(i2c, address, 0x4, 0x00);

  // # Set the REF charging time to 40ms
  reg_write_byte(i2c, address, 0x7b, 0x01);

  // HPL Routing Selection Register
  // route DAC left to HPL
  reg_write_byte(i2c, address, 0x0C, 0x08);

  // HPR Routing Selection Register
  // route DAC right to HPR
  reg_write_byte(i2c, address, 0x0D, 0x08);

  // # HP soft stepping settings for optimal pop performance at power up
  // # Rpop used is 6k with N = 6 and soft step = 20usec. This should work with
  // 47uF coupling # capacitor. Can try N=5,6 or 7 time constants as well.
  // Trade-off delay vs “pop” sound. w 30 14 25
  reg_write_byte(i2c, address, 0x14, 0x25);

  // # Select Page 1
  reg_write_byte(i2c, address, 0x0, 0x1);

  // HPL unmute, set Gain to lowest
  reg_write_byte(i2c, address, 0x10, 0x3B);

  // HPR unmute, set Gain to lowest
  reg_write_byte(i2c, address, 0x11, 0x3B);

  // power up HPL & HPR output
  reg_write_byte(i2c, address, 0x9, 0x30);

  // # Wait for 2.5 sec for soft stepping to take effect
  sleep_ms(2500);

  // ==========================================

  // # Select Page 0
  reg_write_byte(i2c, address, 0x0, 0x00);

  // Left DAC volume control, set to -63.5dB
  reg_write_byte(i2c, address, 0x41, 0x81);
  // Right DAC volume control, set to -63.5dB
  reg_write_byte(i2c, address, 0x42, 0x81);

  // Power up left,right data paths and set
  reg_write_byte(i2c, address, 0x3F, 0xD4);

  // unmute DAC volume, 0 == independent vol for L & R channels
  reg_write_byte(i2c, address, 0x40, 0x0);
  // ==========================================
}

void initI2C() {
  // first need to init Codec via I2C
  // pico SDK i2c read/write function calls use 7bit I2C address
  static const uint8_t TLV320AIC3204_ADDR = 0x18;
  // I2C Port
  i2c_inst_t *i2c = i2c1;

  // Initialize I2C port at 100 kHz
  i2c_init(i2c, 100 * 1000);

  // Initialize I2C pins
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

  // first take codec reset line high to complete its reset
  gpio_init(CODEC_RESET);
  gpio_set_dir(CODEC_RESET, GPIO_OUT);
  gpio_put(CODEC_RESET, 1);
  gpio_put(CODEC_RESET, 0);
  sleep_us(100);
  gpio_put(CODEC_RESET, 1);

  // setup codec over i2c BEFORE sending any i2s data to it!
  setupCodecForDAC(i2c, TLV320AIC3204_ADDR);

  // configure master clock for codec
  uint mclk_offset = pio_add_program(AUDIO_PIO, &audio_codec_i2s_mclk_program);

  audio_codec_i2s_mclk_program_init(AUDIO_PIO, AUDIO_MCLK_SM, mclk_offset,
                                    CODEC_MCLK);

  pio_enable_sm_mask_in_sync(AUDIO_PIO, 0xF);
}
