#ifndef _PICOTRACKEREVENTMANAGER_
#define _PICOTRACKEREVENTMANAGER_

#include "Foundation/T_Singleton.h"
#include "Services/Controllers/KeyboardControllerSource.h"
#include "UIFramework/SimpleBaseClasses/EventManager.h"
#include <string>

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
  virtual void PauseClock() { isClockRunning_ = false; }
  virtual void RunClock() { isClockRunning_ = true; }
  static bool ClockRunning() { return isClockRunning_; }

protected:
  static void ProcessInputEvent();

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
  static bool isClockRunning_;
};
#endif
