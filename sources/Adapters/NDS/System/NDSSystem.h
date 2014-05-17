#ifndef _NDS_SYSTEM_H_
#define _NDS_SYSTEM_H_



#include "System/System/System.h"
#include "System/System/typedefs.h"

class NDSSystem: public System {
public:
	static void Boot() ;
	static void Shutdown() ;
	static int MainLoop() ;

public: // System implementation
	virtual unsigned long GetClock() ;
	virtual void *Malloc(unsigned size) ;
	virtual void Free(void *) ;
    virtual int GetBatteryLevel() ;
    virtual void AddUserLog(const char *) ;
    virtual void PostQuitMessage() ;
    virtual void Memset(void *addr,char val,int size) ;
    virtual void *Memcpy(void *s1, const void *s2, int n)  ;  
protected:
    static void ProcessOneEvent() ;
private:
    static bool finished_ ;
    static bool redrawing_ ;
    static uint16 buttonMask_ ;   
	static unsigned int keyRepeat_ ;
	static unsigned int keyDelay_ ;
	static unsigned int keyKill_ ;
    static bool isRepeating_ ;
    static unsigned long time_ ;
    static bool escPressed_ ;                  
} ;
#endif
