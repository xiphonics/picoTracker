/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"

#define AUDIO_IN_SDATA 18
#define AUDIO_IN_BCLK 19
#define AUDIO_IN_LRCLK 20
#define AUDIO_OUT_SDATA 7
#define AUDIO_OUT_BCLK 8
#define AUDIO_OUT_LRCLK 9
#define AMP_MUTE 13
#define DAC_XSMT 5 // MUTE

int main() {
  stdio_init_all();

  gpio_init(AUDIO_IN_SDATA);
  gpio_set_dir(AUDIO_IN_SDATA, GPIO_IN);
  gpio_init(AUDIO_IN_BCLK);
  gpio_set_dir(AUDIO_IN_BCLK, GPIO_IN);
  gpio_init(AUDIO_IN_LRCLK);
  gpio_set_dir(AUDIO_IN_LRCLK, GPIO_IN);
  gpio_init(AUDIO_OUT_SDATA);
  gpio_set_dir(AUDIO_OUT_SDATA, GPIO_OUT);
  gpio_init(AUDIO_OUT_BCLK);
  gpio_set_dir(AUDIO_OUT_BCLK, GPIO_OUT);
  gpio_init(AUDIO_OUT_LRCLK);
  gpio_set_dir(AUDIO_OUT_LRCLK, GPIO_OUT);

  // Unmute the DAC and output amp
  gpio_init(AMP_MUTE);
  gpio_set_dir(AMP_MUTE, GPIO_OUT);
  gpio_init(DAC_XSMT);
  gpio_set_dir(DAC_XSMT, GPIO_OUT);
  gpio_put(DAC_XSMT, 1);
  gpio_put(AMP_MUTE, 1);

  uint32_t read;
  while (true) {
    // Dummy implementation of the DSP. We bitbang the input i2s signal into the
    // output i2s GPIO. In the future this needs to implement input i2s, output
    // i2s and do something with it!
    read = gpio_get_all();
    gpio_put_masked(0x380, read >> 11);
  }
  return 0;
}
