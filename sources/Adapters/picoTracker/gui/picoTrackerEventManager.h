#ifndef _PICOTRACKEREVENTMANAGER_
#define _PICOTRACKEREVENTMANAGER_
#include "Externals/etl/include/etl/queue.h"
#include "Foundation/T_Singleton.h"
#include "Services/Audio/AudioDriver.h" // for MAX_SAMPLE_COUNT
#include "Services/Audio/AudioModule.h"
#include "Services/Controllers/KeyboardControllerSource.h"
#include "UIFramework/SimpleBaseClasses/EventManager.h"
#include <string>

#include "pico/mutex.h"
#include "pico/stdlib.h"

class picoTrackerEventManager : public T_Singleton<picoTrackerEventManager>,
                                public EventManager {
public:
  picoTrackerEventManager();
  ~picoTrackerEventManager();
  virtual bool Init();
  virtual int MainLoop();
  virtual void PostQuitMessage();
  virtual int GetKeyCode(const char *name);

  static fixed renderBuffer_[MAX_SAMPLE_COUNT * 2];
  static int samplecount;
  static fixed *buffer;
  static mutex_t renderMtx;
  static bool gotData;
  static AudioModule *current;

protected:
  static void ProcessInputEvent();
  void renderAudio();

private:
  static repeating_timer_t timer_;

  static bool finished_;
  static bool redrawing_;
  static uint16_t buttonMask_;
  static unsigned int keyRepeat_;
  static unsigned int keyDelay_;
  static unsigned int keyKill_;
  static bool isRepeating_;
  static unsigned long time_;
  KeyboardControllerSource *keyboardCS_;
};
#endif
