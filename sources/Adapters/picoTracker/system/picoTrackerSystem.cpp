#include "picoTrackerSystem.h"
#include "Adapters/picoTracker/audio/picoTrackerAudio.h"
#include "Adapters/picoTracker/filesystem/picoTrackerFileSystem.h"
#include "Adapters/picoTracker/gui/GUIFactory.h"
#include "Adapters/picoTracker/timer/picoTrackerTimer.h"
#ifdef DUMMY_MIDI
#include "Adapters/Dummy/Midi/DummyMidi.h"
#else
#include "Adapters/picoTracker/midi/picoTrackerMidiService.h"
#endif
#include "Application/Commands/NodeList.h"
#include "Application/Controllers/ControlRoom.h"
#include "Application/Model/Config.h"
#include "Application/Player/SyncMaster.h"
#include "System/Console/Logger.h"
#include "input.h"
#include <assert.h>
#include <fcntl.h>
#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

EventManager *picoTrackerSystem::eventManager_ = NULL;
bool picoTrackerSystem::invert_ = false;
int picoTrackerSystem::lastBattLevel_ = 100;
unsigned int picoTrackerSystem::lastBeatCount_ = 0;

int picoTrackerSystem::MainLoop() {

  eventManager_->InstallMappings();
  return eventManager_->MainLoop();
};

void picoTrackerSystem::Boot(int argc, char **argv) {

  // Install System
  System::Install(new picoTrackerSystem());

  // Install FileSystem
  FileSystem::Install(new picoTrackerFileSystem());
  Path::SetAlias("bin", "");
  Path::SetAlias("root", "");

  Trace::GetInstance()->SetLogger(*(new StdOutLogger()));

  // Install GUI Factory
  I_GUIWindowFactory::Install(new GUIFactory());

  // Install Timers
  TimerService::GetInstance()->Install(new picoTrackerTimerService());

  // Install Sound
  AudioSettings hint;
  hint.bufferSize_ = 1024;
  hint.preBufferCount_ = 8;
  Audio::Install(new picoTrackerAudio(hint));

  // Install Midi
#ifdef DUMMY_MIDI
  MidiService::Install(new DummyMidi());
#else
  MidiService::Install(new picoTrackerMidiService());
#endif

  eventManager_ = I_GUIWindowFactory::GetInstance()->GetEventManager();
  eventManager_->Init();

  bool invert = false;
  Config *config = Config::GetInstance();
  const char *s = config->GetValue("INVERT");

  if ((s) && (!strcmp(s, "YES"))) {
    invert = true;
  }

  if (!invert) {
    eventManager_->MapAppButton("left ctrl", APP_BUTTON_A);
    eventManager_->MapAppButton("left alt", APP_BUTTON_B);
  } else {
    eventManager_->MapAppButton("left alt", APP_BUTTON_A);
    eventManager_->MapAppButton("left ctrl", APP_BUTTON_B);
  }
  eventManager_->MapAppButton("return", APP_BUTTON_START);
  //	em->MapElement("esc",APP_BUTTON_SELECT) ;
  eventManager_->MapAppButton("tab", APP_BUTTON_L);
  eventManager_->MapAppButton("backspace", APP_BUTTON_R);
  eventManager_->MapAppButton("right", APP_BUTTON_RIGHT);
  eventManager_->MapAppButton("left", APP_BUTTON_LEFT);
  eventManager_->MapAppButton("down", APP_BUTTON_DOWN);
  eventManager_->MapAppButton("up", APP_BUTTON_UP);

  adc_init();
  //TODO: use BATT_VOLTAGE_IN_PIN from platform/gpio.h instead of hardcoding pin number here
  gpio_set_dir(29, GPIO_IN);
  printf("ADC INIT DONE\n");

  //  mode0_print("boot successful");
};

void picoTrackerSystem::Shutdown() {
  delete Audio::GetInstance();
};

static int secbase;

unsigned long picoTrackerSystem::GetClock() {
  struct timeval tp;

  gettimeofday(&tp, NULL);
  if (!secbase) {
    secbase = tp.tv_sec;
    return long(tp.tv_usec / 1000.0);
  }
  return long((tp.tv_sec - secbase) * 1000 + tp.tv_usec / 1000.0);
}

#define N_SAMPLES 1
uint16_t sample_buf[N_SAMPLES];

void __not_in_flash_func(adc_capture)(uint16_t *buf, size_t count) {
    adc_fifo_setup(true, false, 0, false, false);
    adc_run(true);
    for (size_t i = 0; i < count; i = i + 1)
        buf[i] = adc_fifo_get_blocking();
    adc_run(false);
    adc_fifo_drain();
}

int picoTrackerSystem::GetBatteryLevel() {

  int lastBattLevel_ = -1;
  unsigned int beatCount = SyncMaster::GetInstance()->GetBeatCount();
  if (beatCount != lastBeatCount_) {
    adc_capture(sample_buf, N_SAMPLES);

    printf("BATTERY: %d", sample_buf[0]);
    lastBattLevel_ = sample_buf[0];

    lastBeatCount_ = beatCount;
  }
  
  return lastBattLevel_;
}


void picoTrackerSystem::Sleep(int millisec) {
  //	if (millisec>0)
  //		assert(0) ;
}

void *picoTrackerSystem::Malloc(unsigned size) {
  void *ptr = malloc(size);
  return ptr;
}

void picoTrackerSystem::Free(void *ptr) { free(ptr); }

void picoTrackerSystem::Memset(void *addr, char val, int size) {
  memset(addr, val, size);
};

void *picoTrackerSystem::Memcpy(void *s1, const void *s2, int n) {
  return memcpy(s1, s2, n);
}

void picoTrackerSystem::PostQuitMessage() { eventManager_->PostQuitMessage(); }

unsigned int picoTrackerSystem::GetMemoryUsage() {
  struct mallinfo m = mallinfo();
  return m.uordblks;
}
