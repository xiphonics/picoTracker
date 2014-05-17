#ifndef _GPSDL_SYSTEM_H_
#define _GPSDL_SYSTEM_H_

#include "System/System/System.h"
#include "UIFramework/SimpleBaseClasses/EventManager.h"
#include <SDL/SDL.h>

#define USEREVENT_TIMER 0
#define USEREVENT_EXPOSE 1

class GPSDLSystem: public System {
public:
	static void Boot(int argc,char **argv) ;
	static void Shutdown() ;
	static int MainLoop() ;

public: // System implementation
	virtual unsigned long GetClock() ;
  virtual int GetBatteryLevel() ;
	virtual void Sleep(int millisec);
	virtual void *Malloc(unsigned size) ;
	virtual void Free(void *) ;
  virtual void Memset(void *addr,char val,int size) ;
  virtual void *Memcpy(void *s1, const void *s2, int n)  ; 
	virtual void PostQuitMessage() ;
	virtual unsigned int GetMemoryUsage() ;

private:
	static bool invert_ ;
	static int lastBattLevel_ ;
	static unsigned int lastBeatCount_ ;
	static EventManager *eventManager_;	
} ;
#endif
