#ifndef _DEB_SYSTEM_H_
#define _DEB_SYSTEM_H_

#include "System/System/System.h"
#include "UIFramework/SimpleBaseClasses/EventManager.h"
#include <SDL/SDL.h>

class DEBSystem: public System {
public:
	static void Boot(int argc,char **argv) ;
	static void Shutdown() ;
	static int MainLoop() ;

public: // System implementation
	virtual unsigned long GetClock() ;
	virtual void Sleep(int millisec);
	virtual void *Malloc(unsigned size) ;
	virtual void Free(void *) ;
    virtual void Memset(void *addr,char val,int size) ;
    virtual void *Memcpy(void *s1, const void *s2, int n)  ; 
	virtual void AddUserLog(const char *) ;
	virtual int GetBatteryLevel() { return -1 ; } ;
	virtual void PostQuitMessage() ;
	virtual unsigned int GetMemoryUsage() ;

protected: // Additional
	static unsigned short getButtonMask(SDL_Event&) ;
	static void readConfig() ;
	static void checkKey(int ,const char *,const char *[]);
private:
	static int keyRepeat_ ;
	static int keyDelay_ ;
	static bool finished_ ;
	static EventManager *eventManager_;
} ;
#endif
