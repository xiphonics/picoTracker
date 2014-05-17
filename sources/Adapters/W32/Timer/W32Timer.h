#ifndef _W32_TIMER_H_
#define _W32_TIMER_H_

#include "System/Timer/Timer.h"
#include <windows.h>

class W32Timer: public I_Timer {
public:
	W32Timer() ;
	virtual ~W32Timer() ;
	virtual void SetPeriod(float msec) ;
	virtual bool Start() ;
	virtual void Stop() ;
	virtual float GetPeriod() ;
	void OnTimerTick() ;
private:
	float period_ ;
	float offset_ ; // Float offset taking into account
					// period is an int
	MMRESULT timer_ ; // NULL if not running
	long lastTick_ ;
	bool running_ ;
} ;

class W32TimerService: public TimerService {
public:
	virtual I_Timer *CreateTimer() ; // Returns a timer
	virtual void TriggerCallback(int msec,timerCallback cb) ;
};

#endif