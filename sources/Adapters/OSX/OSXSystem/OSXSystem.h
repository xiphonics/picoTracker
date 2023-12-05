#ifndef _OSX_SYSTEM_H_
#define _OSX_SYSTEM_H_

#include "System/System/System.h"
#include "UIFramework/SimpleBaseClasses/EventManager.h"

// TODO: Check how I can have a single place for all SDL based code
// So I don't have to check all of the System::MainLoop()

class OSXSystem : public System {
public:
  static void Boot(int argc, char **argv);
  static void Shutdown();
  static int MainLoop();

public: // System implementation
  virtual unsigned long GetClock();
  virtual void Sleep(int millisec);
  virtual void *Malloc(unsigned size);
  virtual void Free(void *);
  virtual void Memset(void *addr, char val, int size);
  virtual void *Memcpy(void *s1, const void *s2, int n);
  virtual int GetBatteryLevel() { return -1; };
  virtual void PostQuitMessage();
  virtual unsigned int GetMemoryUsage();
  static bool finished_;

protected:
  static void installAliases();

private:
  static EventManager *eventManager_;
};
#endif
