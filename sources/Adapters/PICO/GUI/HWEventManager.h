#ifndef _HW_EVENT_MANAGER_
#define _HW_EVENT_MANAGER_

#include "Foundation/T_Singleton.h"
#include "Services/Controllers/KeyboardControllerSource.h"
#include "UIFramework/SimpleBaseClasses/EventManager.h"
#include <string>

#include "pico/stdlib.h"

class HWEventManager : public T_Singleton<HWEventManager>, public EventManager {
public:
  HWEventManager();
  ~HWEventManager();
  virtual bool Init();
  virtual int MainLoop();
  virtual void PostQuitMessage();
  virtual int GetKeyCode(const char *name);

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
  static bool escPressed_;
  KeyboardControllerSource *keyboardCS_;
};
#endif
