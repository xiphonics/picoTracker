#include "utils.h"
#include "System/System/System.h"
#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include <stdio.h>

uint32_t millis(void) { return to_ms_since_boot(get_absolute_time()); }

uint32_t micros(void) { return to_us_since_boot(get_absolute_time()); }

uint32_t measure_free_mem(void) {
  void *buff[256];
  uint32_t max = 0;

  int i = 0;
  for (; i < 256; i++) {
    buff[i] = malloc(1000);
    if (buff[i]) {
      max = i;
    } else {
      break;
    }
  }
  for (int j = i; j >= 0; j--) {
    free(buff[j]);
  }

  printf("MAX memory free in heap: %i\n", max * 1000);
  /*
    buff = malloc(80000);
  if (buff) {
    printf("MALLOC addr: %p %i - Mem free: %i\n", buff,
           reinterpret_cast<uintptr_t>(buff),  0x20040000l -
               reinterpret_cast<uintptr_t>(buff));
    free(buff);
    }*/
  return max;
}

void measure_freqs(void) {
  uint f_pll_sys =
      frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY);
  uint f_pll_usb =
      frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_USB_CLKSRC_PRIMARY);
  uint f_rosc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_ROSC_CLKSRC);
  uint f_clk_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
  uint f_clk_peri = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_PERI);
  uint f_clk_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_USB);
  uint f_clk_adc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_ADC);
  uint f_clk_rtc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_RTC);

  printf("pll_sys  = %dkHz\n", f_pll_sys);
  printf("pll_usb  = %dkHz\n", f_pll_usb);
  printf("rosc     = %dkHz\n", f_rosc);
  printf("clk_sys  = %dkHz\n", f_clk_sys);
  printf("clk_peri = %dkHz\n", f_clk_peri);
  printf("clk_usb  = %dkHz\n", f_clk_usb);
  printf("clk_adc  = %dkHz\n", f_clk_adc);
  printf("clk_rtc  = %dkHz\n", f_clk_rtc);

  // Can't measure clk_ref / xosc as it is the ref
}
