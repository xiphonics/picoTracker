/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Code taken from pico-extras dormant mode helper functions

#include "rpi_helpers.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"
#include "hardware/regs/clocks.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/watchdog.h"
#include "hardware/xosc.h"
#include "pico/stdlib.h"
#include "rosc.h"

// for clocks_init()
#include "pico/runtime_init.h"

#include "hardware/powman.h"

// The difference between sleep and dormant is that ALL clocks are stopped in
// dormant mode, until the source (either xosc or rosc) is started again by an
// external event. In sleep mode some clocks can be left running controlled by
// the SLEEP_EN registers in the clocks block. For example you could keep
// clk_rtc running. Some destinations (proc0 and proc1 wakeup logic) can't be
// stopped in sleep mode otherwise there wouldn't be enough logic to wake up
// again.

static dormant_source_t _dormant_source;

static void _go_dormant(void) {

  if (_dormant_source == DORMANT_SOURCE_XOSC) {
    xosc_dormant();
  } else {
    rosc_set_dormant();
  }
}

// In order to go into dormant mode we need to be running from a stoppable clock
// source: either the xosc or rosc with no PLLs running. This means we disable
// the USB and ADC clocks and all PLLs
void sleep_run_from_dormant_source(dormant_source_t dormant_source) {
  _dormant_source = dormant_source;

  uint src_hz;
  uint clk_ref_src;
  switch (dormant_source) {
  case DORMANT_SOURCE_XOSC:
    src_hz = XOSC_HZ;
    clk_ref_src = CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC;
    break;
  case DORMANT_SOURCE_ROSC:
    src_hz = 6500 * KHZ; // todo
    clk_ref_src = CLOCKS_CLK_REF_CTRL_SRC_VALUE_ROSC_CLKSRC_PH;
    break;
#if !PICO_RP2040
  case DORMANT_SOURCE_LPOSC:
    src_hz = 32 * KHZ;
    clk_ref_src = CLOCKS_CLK_REF_CTRL_SRC_VALUE_LPOSC_CLKSRC;
    break;
#endif
  default:
    hard_assert(false);
  }

  // CLK_REF = XOSC or ROSC
  clock_configure(clk_ref, clk_ref_src,
                  0, // No aux mux
                  src_hz, src_hz);

  // CLK SYS = CLK_REF
  clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLK_REF,
                  0, // Using glitchless mux
                  src_hz, src_hz);

  // CLK ADC = 0MHz
  clock_stop(clk_adc);
  clock_stop(clk_usb);
  clock_stop(clk_hstx);

#if PICO_RP2040
  // CLK RTC = ideally XOSC (12MHz) / 256 = 46875Hz but could be rosc
  uint clk_rtc_src = (dormant_source == DORMANT_SOURCE_XOSC)
                         ? CLOCKS_CLK_RTC_CTRL_AUXSRC_VALUE_XOSC_CLKSRC
                         : CLOCKS_CLK_RTC_CTRL_AUXSRC_VALUE_ROSC_CLKSRC_PH;

  clock_configure(clk_rtc,
                  0, // No GLMUX
                  clk_rtc_src, src_hz, 46875);
#endif

  // CLK PERI = clk_sys. Used as reference clock for Peripherals. No dividers so
  // just select and enable
  clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                  src_hz, src_hz);

  pll_deinit(pll_sys);
  pll_deinit(pll_usb);

  // Assuming both xosc and rosc are running at the moment
  if (dormant_source == DORMANT_SOURCE_XOSC) {
    // Can disable rosc
    rosc_disable();
  } else {
    // Can disable xosc
    xosc_disable();
  }

  // Reconfigure uart with new clocks
  setup_default_uart();
}

void ptsleep_goto_dormant_until_pin(uint8_t gpio_pin, bool edge, bool high) {
  bool low = !high;
  bool level = !edge;

  // Configure the appropriate IRQ at IO bank 0
  assert(gpio_pin < NUM_BANK0_GPIOS);

  uint32_t event = 0;

  if (level && low)
    event = IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_LEVEL_LOW_BITS;
  if (level && high)
    event = IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_LEVEL_HIGH_BITS;
  if (edge && high)
    event = IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_EDGE_HIGH_BITS;
  if (edge && low)
    event = IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_EDGE_LOW_BITS;

  gpio_init(gpio_pin);
  gpio_set_input_enabled(gpio_pin, true);
  gpio_set_dormant_irq_enabled(gpio_pin, event, true);

  _go_dormant();
  // Execution stops here until woken up

  // Clear the irq so we can go back to dormant mode again if we want
  gpio_acknowledge_irq(gpio_pin, event);
  gpio_set_input_enabled(gpio_pin, false);
}

// To be called after waking up from sleep/dormant mode to restore system clocks
// properly
void sleep_power_up(void) {
  // Re-enable the ring oscillator, which will essentially kickstart the proc
  rosc_enable();

  // Reset the sleep enable register so peripherals and other hardware can be
  // used
  clocks_hw->sleep_en0 |= ~(0u);
  clocks_hw->sleep_en1 |= ~(0u);

  // Restore all clocks
  clocks_init();

  // make powerman use xosc again
  // uint64_t restore_ms = powman_timer_get_ms();
  // powman_timer_set_1khz_tick_source_xosc();
  // powman_timer_set_ms(restore_ms);

  // UART needs to be reinitialised with the new clock frequencies for stable
  // output
  setup_default_uart();
}
