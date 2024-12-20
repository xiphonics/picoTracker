#include "picoTrackerSystem.h"
#include "Adapters/picoTracker/audio/picoTrackerAudio.h"
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
#include "hardware/gpio.h"
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

#include "Adapters/picoTracker/platform/platform.h"
#include "critical_error_message.h"
#include "hardware/adc.h"
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
  static char systemMemBuf[sizeof(picoTrackerSystem)];
  System::Install(new (systemMemBuf) picoTrackerSystem());

  // Install GUI Factory
  static char guiMemBuf[sizeof(GUIFactory)];
  I_GUIWindowFactory::Install(new (guiMemBuf) GUIFactory());

  // Install Timers
  static char timerMemBuf[sizeof(picoTrackerTimerService)];
  TimerService::GetInstance()->Install(new (timerMemBuf)
                                           picoTrackerTimerService());

  // First check for SDCard
  auto picoFS = PicoFileSystem::GetInstance();
  if (!picoFS->chdir("/")) {
    Trace::Log("PICOTRACKERSYSTEM", "SDCARD MISSING!!\n");
    critical_error_message("SDCARD MISSING", 0x01);
  }

  // Install MIDI
  // **NOTE**: MIDI install MUST happen before Audio install because it triggers
  // reading config file and config file needs to have MidiService already
  // installed in order to apply midi settings read from the config file
#ifdef DUMMY_MIDI
  static char midiMemBuf[sizeof(DummyMidi)];
  MidiService::Install(new (midiMemBuf) DummyMidi());
#else
  static char midiMemBuf[sizeof(picoTrackerMidiService)];
  MidiService::Install(new (midiMemBuf) picoTrackerMidiService());
#endif

  // Install Sound
  AudioSettings hint;
  hint.bufferSize_ = 1024;
  hint.preBufferCount_ = 8;
  static char audioMemBuf[sizeof(picoTrackerAudio)];
  Audio::Install(new (audioMemBuf) picoTrackerAudio(hint));

  eventManager_ = I_GUIWindowFactory::GetInstance()->GetEventManager();
  eventManager_->Init();

  bool invert = false;
  Config *config = Config::GetInstance();
  invert = config->GetValue("INVERT") > 0;

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

#if PICO_RP2040
  // init GPIO for use as ADC: hi-Z, no pullups, etc
  adc_gpio_init(BATT_VOLTAGE_IN);

  adc_init();

  // select analog MUX, GPIO 26=0, 27=1, 28=1, 29=3
  adc_select_input(3);

  Trace::Log("PICOTRACKERSYSTEM", "ADC INIT DONE\n");
#endif
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

  //  u_int16_t adc_reading = adc_read(); // raw voltage from ADC

  //  int adc_voltage = adc_reading * 0.8; // 0.8mV per unit of ADC
  // *2 because picoTracker use voltage divider for voltage on ADC pin
  //  lastBattLevel_ = adc_voltage * 2;
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
