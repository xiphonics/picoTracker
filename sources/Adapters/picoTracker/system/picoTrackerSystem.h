#ifndef _PICOTRACKERSYSTEM_H_
#define _PICOTRACKERSYSTEM_H_

#include <map>

#include "System/System/System.h"
#include "UIFramework/SimpleBaseClasses/EventManager.h"

#define USEREVENT_TIMER 0
#define USEREVENT_EXPOSE 1

class picoTrackerSystem : public System {
public:
  static void Boot(int argc, char **argv);
  static void Shutdown();
  static int MainLoop();

public: // System implementation
  virtual unsigned long GetClock();
  virtual int GetBatteryLevel();
  virtual void SetDisplayBrightness(unsigned char value);
  virtual void Sleep(int millisec);
  virtual void *Malloc(unsigned size);
  virtual void Free(void *);
  virtual void Memset(void *addr, char val, int size);
  virtual void *Memcpy(void *s1, const void *s2, int n);
  virtual void PostQuitMessage();
  virtual unsigned int GetMemoryUsage();

private:
  static bool invert_;
  static int lastBattLevel_;
  static unsigned int lastBeatCount_;

  // Update brightness from config
  static void UpdateBrightnessFromConfig();

  static EventManager *eventManager_;
  std::map<void *, unsigned> mmap_;
};
#endif
