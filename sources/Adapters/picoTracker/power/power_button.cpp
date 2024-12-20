#include "power_button.h"
#include "System/Console/Trace.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "rpi_helpers.h"
#include <gpio.h>

static volatile bool powerpressed = false;

// forward declaration of "private" functions
void dormantnow(uint wakepin);
void setpowerbuttonirqhandler(uint wake_pin);
// ====

void handlePowerButton() {
  if (powerpressed) {
    Trace::Debug("Power button pressed\n");
    gpio_put(DISPLAY_PWM, 0);
    // need short delay for display pin to go low before going dormant
    busy_wait_ms(100);
    dormantnow(0);
    powerpressed = false;
    setpowerbuttonirqhandler(0);
    gpio_put(DISPLAY_PWM, 1);
  }
}

void powerbuttonirq(uint gpio, uint32_t event_mask) {
  powerpressed = true;
  Trace::Debug("==========> POWER BUTTON pressed!");
  // Clear the irq
  gpio_acknowledge_irq(gpio, event_mask);
}

void dormantnow(uint wake_pin) {
  Trace::Debug("Switching to XOSC\n");
  uart_default_tx_wait_blocking();

  // UART will be reconfigured by sleep_run_from_xosc
  // sleep_run_from_xosc();
  sleep_run_from_dormant_source(DORMANT_SOURCE_XOSC);

  Trace::Debug("Running from XOSC\n");
  uart_default_tx_wait_blocking();

  Trace::Debug("XOSC going dormant\n");
  uart_default_tx_wait_blocking();

  // Go to sleep until we see a high edge on GPIO 0
  ptsleep_goto_dormant_until_pin(wake_pin, true, false);

  sleep_power_up();

  // reenable as coming out dormant call from pico-extras above causes it to
  // be disabled
  gpio_set_input_enabled(wake_pin, true);
  Trace::Debug("POWER UP after going dormant\n");
}

void setpowerbuttonirqhandler(uint wake_pin) {
  gpio_set_irq_enabled_with_callback(wake_pin, GPIO_IRQ_EDGE_FALL, true,
                                     powerbuttonirq);
}
