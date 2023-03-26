#include "platform.h"
#include "hardware/clocks.h"
//#include "hardware/dma.h"
#include "hardware/gpio.h"
//#include "hardware/irq.h"
#include "hardware/pll.h"
#include "hardware/spi.h"
#include "hardware/structs/clocks.h"
#include "hardware/uart.h"
#include "hardware/vreg.h"
#include "pico/stdlib.h"
#include <cstdio>

void platform_init() {
  // Platform setup
  stdio_init_all();

  ////////////
  // CLOCKS //
  ////////////

  // frequencies setup
  // clk_usb has to be 48MHz
  // clk_peri will be 96MHz
  // clk_sys will be 220.5 MHz, which is divisible by 44100 in order to have a
  // Set PLL_USB 96MHz
  pll_init(pll_usb, 1, 1536 * MHZ, 4, 4);
  clock_configure(clk_usb, 0, CLOCKS_CLK_USB_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                  96 * MHZ, 48 * MHZ);
  // Change clk_sys to be 96MHz, depending on USB PLL in order to reconfigure SYS_PLL
  clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
                  CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB, 96 * MHZ,
                  96 * MHZ);
  // set clk_peri to be clocked from USB PLL
  clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                  96 * MHZ, 96 * MHZ);
  // Review this, this limits the SD SPI clock to 24MHz since 48MHz doesn't work
  // Using the stock freq of 125MHz SD SPI can work at 31.25MHz
  
  /*
    All possible frequecies that could be used and are compatible with an exact i2s clock
    35.28 REFDIV: 2 FBDIV: 147 (VCO: 882.0 MHz) PD1: 5 PD2: 5
    44.1 REFDIV: 2 FBDIV: 147 (VCO: 882.0 MHz) PD1: 5 PD2: 4
    55.125 REFDIV: 2 FBDIV: 147 (VCO: 882.0 MHz) PD1: 4 PD2: 4
    88.2 REFDIV: 2 FBDIV: 147 (VCO: 882.0 MHz) PD1: 5 PD2: 2
    110.25 REFDIV: 2 FBDIV: 147 (VCO: 882.0 MHz) PD1: 4 PD2: 2
    176.4 REFDIV: 2 FBDIV: 147 (VCO: 882.0 MHz) PD1: 5 PD2: 1 (overclocked)
    220.5 REFDIV: 2 FBDIV: 147 (VCO: 882.0 MHz) PD1: 4 PD2: 1 (overclocked)
  */
  // Reconfigure SYS PLL to desired frequency
  pll_init(pll_sys, 2, 882 * MHZ, 4, 1); // 220.5 MHz

  // Finally set clk_sys to be clocked from SYS PLL
  clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
                  CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, 220.5 * MHZ,
                  220.5 * MHZ);

  // Other possible frequencies to improve performance at the cost of inexact i2s clock divider
  // all within 0.001 of exact divider
  //  vreg_set_voltage(VREG_VOLTAGE_1_10);
  //  pll_init(pll_sys, 1, 1596 * MHZ, 6, 1);
  //   clock_configure(clk_sys,
  //   CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
  //                    CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, 266 *
  //                    MHZ, 266 * MHZ);

  //    vreg_set_voltage(VREG_VOLTAGE_1_10);
  //    pll_init(pll_sys, 2, 1422 * MHZ, 5, 1);
  //    clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
  //                    CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
  //                    284.4 * MHZ, 284.4 * MHZ);

  //      vreg_set_voltage(VREG_VOLTAGE_1_15);
  //    pll_init(pll_sys, 2, 1458 * MHZ, 5, 1);
  //   clock_configure(clk_sys,
  //    CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
  //                    CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
  //                    291.6 * MHZ, 291.6 * MHZ);

  // Pretty well tested on one particular board, ran for 60 minutes
  // vreg_set_voltage(VREG_VOLTAGE_1_20);
  // pll_init(pll_sys, 2, 1494 * MHZ, 5, 1);
  // clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
  //                 CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
  //                 298.8 * MHZ, 298.8 * MHZ);


   // Reinit uart now that clk_peri has changed
   stdio_init_all();

   ///////////
   // DEBUG //
   ///////////

   /////////////
   // DISPLAY //
   /////////////
   /*

  (pin 1) VCC        5V/3.3V power input
  (pin 2) GND        Ground
  (pin 3) CS         LCD chip select signal, low level enable
  (pin 4) RESET      LCD reset signal, low level reset
  (pin 5) DC/RS      LCD register / data selection signal; high level:
  register, low level: data (pin 6) SDI(MOSI)  SPI bus write data signal (pin
  7) SCK SPI bus clock signal (pin 8) LED        Backlight control; if not
  controlled, connect 3.3V always bright (pin 9) SDO(MISO)  SPI bus read data
  signal; optional

  */

   // This example will use SPI0 at 0.5MHz.
   spi_init(DISPLAY_SPI, 500 * 1000);
   int baudrate = spi_set_baudrate(DISPLAY_SPI, 75000 * 1000);
   printf("Display SPI Baudrate: %i\n", baudrate);

   gpio_set_function(DISPLAY_MISO, GPIO_FUNC_SPI);
   gpio_set_function(DISPLAY_SCK, GPIO_FUNC_SPI);
   gpio_set_function(DISPLAY_MOSI, GPIO_FUNC_SPI);

   // Chip select is active-low, so we'll initialise it to a driven-high state
   gpio_init(DISPLAY_CS);
   gpio_set_dir(DISPLAY_CS, GPIO_OUT);
   gpio_put(DISPLAY_CS, 0);

   // Reset is active-low
   gpio_init(DISPLAY_RESET);
   gpio_set_dir(DISPLAY_RESET, GPIO_OUT);
   gpio_put(DISPLAY_RESET, 1);

   // high = command, low = data
   gpio_init(DISPLAY_DC);
   gpio_set_dir(DISPLAY_DC, GPIO_OUT);
   gpio_put(DISPLAY_DC, 0);

   ////////////
   // SDCARD //
   ////////////

   ///////////
   // AUDIO //
   ///////////

   //////////
   // MIDI //
   //////////
   gpio_set_function(MIDI_OUT_PIN, GPIO_FUNC_UART);
   gpio_set_function(MIDI_IN_PIN, GPIO_FUNC_UART);

   // Set up our UART with the required speed.
   baudrate = uart_init(MIDI_UART, MIDI_BAUD_RATE);
   printf("Init MIDI device with %i baud rate\n", baudrate);

   ///////////
   // INPUT //
   ///////////

   gpio_init(INPUT_COL1);
   gpio_set_dir(INPUT_COL1, GPIO_IN);
   gpio_init(INPUT_COL2);
   gpio_set_dir(INPUT_COL2, GPIO_IN);
   gpio_init(INPUT_COL3);
   gpio_set_dir(INPUT_COL3, GPIO_IN);
   gpio_init(INPUT_ROW1);
   gpio_set_dir(INPUT_ROW1, GPIO_IN);
   gpio_init(INPUT_ROW2);
   gpio_set_dir(INPUT_ROW2, GPIO_IN);
   gpio_init(INPUT_ROW3);
   gpio_set_dir(INPUT_ROW3, GPIO_IN);

   gpio_pull_up(INPUT_COL1);
   gpio_pull_up(INPUT_COL2);
   gpio_pull_up(INPUT_COL3);
   gpio_pull_down(INPUT_ROW1);
   gpio_pull_down(INPUT_ROW2);
   gpio_pull_down(INPUT_ROW3);
}
