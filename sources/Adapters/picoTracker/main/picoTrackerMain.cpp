#include "Adapters/picoTracker/platform/platform.h"
#include "Adapters/picoTracker/system/picoTrackerSystem.h"
#include "Application/Application.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"
#include "pico/stdlib.h"

int main(int argc, char *argv[]) {

  platform_init();

  picoTrackerSystem::Boot(argc, argv);

  GUICreateWindowParams params;
  params.title = "picoTracker";

  Application::GetInstance()->Init(params);

  picoTrackerSystem::MainLoop();
  printf("Finish main loop?\n");

  picoTrackerSystem::Shutdown();
}
