#ifndef _SDL_TIMER_H_
#define _SDL_TIMER_H_

#include "System/Timer/Timer.h"
#include "SDL/SDL.h"

class SDLTimer: public I_Timer {
public:
	SDLTimer() ;
	virtual ~SDLTimer() ;
	virtual void SetPeriod(float msec) ;
	virtual bool Start() ;
	virtual void Stop() ;
	virtual float GetPeriod() ;
	Uint32 OnTimerTick() ;
	
private:
	float period_ ;
	float offset_ ; // Float offset taking into account
					// period is an int
	SDL_TimerID timer_ ; // NULL if not running
	long lastTick_ ;
	bool running_ ;
} ;

class SDLTimerService: public TimerService {
public:
	virtual I_Timer *CreateTimer() ; // Returns a timer
	virtual void TriggerCallback(int msec,timerCallback cb) ;
};

#endif