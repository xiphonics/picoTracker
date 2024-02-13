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

#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

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

  // init GPIO for use as ADC: hi-Z, no pullups, etc
  adc_gpio_init(29);

  adc_init();

  // select analog MUX, GPIO 26=0, 27=1, 28=1, 29=3
  adc_select_input(3);

  // TODO: use BATT_VOLTAGE_IN_PIN from platform/gpio.h instead of hardcoding
  // pin number here
  printf("ADC INIT DONE\n");

  //  mode0_print("boot successful");
};

void picoTrackerSystem::Shutdown() { delete Audio::GetInstance(); };

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

int picoTrackerSystem::GetBatteryLevel() {

  int lastBattLevel_ = -1;
  unsigned int beatCount = SyncMaster::GetInstance()->GetBeatCount();
  if (beatCount != lastBeatCount_) {
    u_int16_t adc_reading = adc_read(); // raw voltage from ADC
    printf("ADC READING: %d ", adc_reading);

    int adc_voltage = adc_reading * 0.8; // 0.8mV per unit of ADC

    // *2 because picoTracker use voltage divider for voltage on ADC pin
    lastBattLevel_ = adc_voltage * 2;
    
    printf("BATTERY: %d ", lastBattLevel_);

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
