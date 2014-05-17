
#include "SDLTimer.h"
#include "System/Console/Trace.h"
#include "System/System/System.h"
#include "System/Console/n_assert.h"

Uint32 SDLTimerCallback(Uint32 interval, void* param) {
	SDLTimer *timer=(SDLTimer *)param ;
	return timer->OnTimerTick() ;
} ;

Uint32 SDLTriggerCallback(Uint32 interval, void* param) {
	timerCallback tc=(timerCallback)param ;
	(*tc)() ;
	return 0 ;
} ;

SDLTimer::SDLTimer() {
	period_=-1 ;
	timer_=0 ;
	running_=false ;
} ;

SDLTimer::~SDLTimer() {
}

void SDLTimer::SetPeriod(float msec) {
	period_=int(msec) ;
	offset_=0 ;
} ;

bool SDLTimer::Start() {
	if (period_>0) {
		offset_=period_ ;
		Uint32 newcb=int(offset_) ;
		offset_-=newcb ;
		timer_=SDL_AddTimer(newcb,SDLTimerCallback,this);
		lastTick_=System::GetInstance()->GetClock() ;
		running_=true ;
	}
	return (timer_!=0) ;
} ;

void SDLTimer::Stop() {
	SDL_RemoveTimer(timer_) ;
	timer_=0 ;
	running_=false ;
} ;

float SDLTimer::GetPeriod() {
	return period_ ;
} ;

Uint32 SDLTimer::OnTimerTick() {
	Uint32 newcb=0 ;
	if (running_) {
		SetChanged() ;
		NotifyObservers() ;
		offset_+=period_ ;
		newcb=int(offset_) ;
		offset_-=newcb ;
		NAssert(newcb>0) ;
	}
	return newcb ;
} ;

I_Timer *SDLTimerService::CreateTimer() {
	return new SDLTimer() ;
} ;

void SDLTimerService::TriggerCallback(int msec,timerCallback cb) {
	SDL_AddTimer(msec,SDLTriggerCallback,(void *)cb);
}
