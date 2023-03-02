#include "Adapters/PICO/System/PICOSystem.h"
#include "Application/Application.h"
// #include "Foundation/T_Singleton.h"
// #include <string.h>
// #include "Adapters/SDL/GUI/SDLGUIWindowImp.h"
// #include "Application/Persistency/PersistencyService.h"
// #include "Adapters/SDL/GUI/SDLGUIWindowImp.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"
#include "pico/stdlib.h"

// TODO: Check if this is necessary for i2s
static const uint32_t PIN_DCDC_PSM_CTRL = 23;

int main(int argc, char *argv[]) {
  
  // Platform setup
  stdio_init_all();
  
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
    176.4 REFDIV: 2 FBDIV: 147 (VCO: 882.0 MHz) PD1: 5 PD2: 1
    220.5 REFDIV: 2 FBDIV: 147 (VCO: 882.0 MHz) PD1: 4 PD2: 1
  */
  // Reconfigure SYS PLL to desired frequency
  pll_init(pll_sys, 2, 882 * MHZ, 4, 1); // 220.5 MHz
  // Finally set clk_sys to be clocked from SYS PLL
  clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
                  CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, 220.5 * MHZ,
                  220.5 * MHZ);

  // Reinit uart now that clk_peri has changed
  stdio_init_all();

  // i2s settings
  // DCDC PSM control
  // 0: PFM mode (best efficiency)
  // 1: PWM mode (improved ripple)
  gpio_init(PIN_DCDC_PSM_CTRL);
  gpio_set_dir(PIN_DCDC_PSM_CTRL, GPIO_OUT);
  gpio_put(PIN_DCDC_PSM_CTRL, 1); // PWM mode for less Audio noise

  PICOSystem::Boot(argc, argv);

  GUICreateWindowParams params;
  params.title = "littlegptracker";

  Application::GetInstance()->Init(params);

  PICOSystem::MainLoop();
  printf("Finish main loop?\n");

  PICOSystem::Shutdown();
}
