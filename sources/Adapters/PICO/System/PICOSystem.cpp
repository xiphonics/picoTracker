#include "PICOSystem.h"
#include "Adapters/PICO/FileSystem/PICOFileSystem.h"
// #include "Adapters/SDL/GUI/GUIFactory.h"
#include "Adapters/PICO/GUI/GUIFactory.h"
// #include "Adapters/SDL/GUI/SDLGUIWindowImp.h"
// #include "Adapters/SDL/GUI/SDLEventManager.h"
#include "Adapters/PICO/Process/HWProcess.h"
#ifdef DUMMY_AUDIO
#include "Adapters/Dummy/Audio/DummyAudio.h"
#else
#include "Adapters/PICO/Audio/PICOAudio.h"
#endif
#include "Adapters/PICO/Midi/PICOMidiService.h"
#include "Adapters/PICO/Timer/Timer.h"
#include "Application/Commands/NodeList.h"
#include "Application/Controllers/ControlRoom.h"
#include "Application/Model/Config.h"
#include "Application/Player/SyncMaster.h"
#include "System/Console/Logger.h"
#include <assert.h>
#include <fcntl.h>
#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
// #include "Externals/FreeRTOS/FreeRTOS-Kernel/include/FreeRTOS.h"
// #include "Externals/FreeRTOS/FreeRTOS-Kernel/include/task.h"
#include "input.h"

extern "C" {
#include "Adapters/PICO/ili9341/mode0.h"
}

EventManager *PICOSystem::eventManager_ = NULL;
bool PICOSystem::invert_ = false;
int PICOSystem::lastBattLevel_ = 100;
unsigned int PICOSystem::lastBeatCount_ = 0;

static FILE *devbatt_;
static char *strval;
static size_t n;
static size_t t;

int PICOSystem::MainLoop() {

  eventManager_->InstallMappings();
  return eventManager_->MainLoop();
};

void PICOSystem::Boot(int argc, char **argv) {

  inputInit();

  // TODO: check where this should go
  //  vTaskStartScheduler();

  // Install System
  System::Install(new PICOSystem());

  // Install FileSystem
  FileSystem::Install(new PICOFileSystem());
  Path::SetAlias("bin", "");
  Path::SetAlias("root", "");

  // TODO: cleanup this
  /*	Path::SetAlias("bin",".") ;
        Path::SetAlias("root",".") ;

        Config::GetInstance()->ProcessArguments(argc,argv) ;
  */
  // #ifdef _DEBUG
  Trace::GetInstance()->SetLogger(*(new StdOutLogger()));
  // #endif
  /*
#ifdef _DEBUG
  Trace::GetInstance()->SetLogger(*(new StdOutLogger()));
#else
  Path logPath("bin:lgpt.log");
  FileLogger *fileLogger=new FileLogger(logPath);
  if(fileLogger->Init().Succeeded())
  {
    Trace::GetInstance()->SetLogger(*fileLogger);
  }
#endif
  */

  // Install GUI Factory
  //  mode0_print("GUI init\n");
  I_GUIWindowFactory::Install(new GUIFactory());

  // Install Timers
  //  mode0_print("Timer init\n");
  TimerService::GetInstance()->Install(new HWTimerService());

  // Install Sound
  //  mode0_print("Audio init\n");
  AudioSettings hint;
  hint.bufferSize_ = 1024;
  hint.preBufferCount_ = 8;
#ifdef DUMMY_AUDIO
  Audio::Install(new DummyAudio(hint));
#else
  Audio::Install(new PICOAudio(hint));
#endif

  // Install Midi
  //  mode0_print("MIDI init\n");
  MidiService::Install(new PICOMidiService());

  // Install Threads
  //  mode0_print("Threads init\n");
  SysProcessFactory::Install(new HWProcessFactory());

  //  mode0_print("EventManager init\n");
  //  printf("EventManager init\n");
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
  //  mode0_print("boot successful");
};

void PICOSystem::Shutdown() {
  delete Audio::GetInstance();
  fclose(devbatt_);
};

static int secbase;

unsigned long PICOSystem::GetClock() {
  struct timeval tp;

  gettimeofday(&tp, NULL);
  if (!secbase) {
    secbase = tp.tv_sec;
    return long(tp.tv_usec / 1000.0);
  }
  return long((tp.tv_sec - secbase) * 1000 + tp.tv_usec / 1000.0);
}

int PICOSystem::GetBatteryLevel() {

  if (0) {
    unsigned int beatCount = SyncMaster::GetInstance()->GetBeatCount();
    if (beatCount != lastBeatCount_) {
      unsigned short currentval = 0;
      fseek(devbatt_, 0, SEEK_SET);
      fread(strval, t, n, devbatt_);
      currentval = atoi(strval);
      if (currentval > 4000)
        currentval = 4000;
      if (currentval < 3000)
        currentval = 3000;
      lastBattLevel_ = ((currentval - 3000) / 10);
      lastBeatCount_ = beatCount;
    }
  } else {
    lastBattLevel_ = -1;
  }
  return lastBattLevel_;
};

void PICOSystem::Sleep(int millisec) {
  //	if (millisec>0)
  //		assert(0) ;
}

void *PICOSystem::Malloc(unsigned size) {
  void *ptr = malloc(size);
  /*
  Trace::Debug("MALLOC size: %i, ptr: 0x%X", size, ptr);
  mmap_.insert(std::pair<void *, unsigned>(ptr, size));
  unsigned total=0;
  for (auto const& [key, val] : mmap_)
    {
      total += val;
    }
    Trace::Debug("MALLOC total memory used: %i", total);*/
  return ptr;
}

void PICOSystem::Free(void *ptr) {
  //  Trace::Debug("Free ptr: 0x%X", ptr);
  //  mmap_.erase(ptr);
  free(ptr);
}

// extern "C" void *gpmemset(void *s1,unsigned char val,int n) ;

void PICOSystem::Memset(void *addr, char val, int size) {
  memset(addr, val, size);
};

// extern "C" void *gpmemcpy(void *s1, const void *s2, int n) ;

void *PICOSystem::Memcpy(void *s1, const void *s2, int n) {
  return memcpy(s1, s2, n);
}

void PICOSystem::PostQuitMessage() { eventManager_->PostQuitMessage(); }

unsigned int PICOSystem::GetMemoryUsage() {
  struct mallinfo m = mallinfo();
  return m.uordblks;
}
