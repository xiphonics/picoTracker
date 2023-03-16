#include "Adapters/PICO/System/PICOSystem.h"
#include "Application/Application.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"
#include "pico/stdlib.h"
#include "Adapters/PICO/platform/platform.h"

int main(int argc, char *argv[]) {

  platform_init();

  PICOSystem::Boot(argc, argv);

  GUICreateWindowParams params;
  params.title = "littlegptracker";

  Application::GetInstance()->Init(params);

  PICOSystem::MainLoop();
  printf("Finish main loop?\n");

  PICOSystem::Shutdown();
}
