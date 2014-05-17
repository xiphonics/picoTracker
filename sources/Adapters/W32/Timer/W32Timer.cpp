
#include "W32Timer.h"
#include "System/Console/Trace.h"
#include "System/System/System.h"
#include <assert.h>

static void CALLBACK TimerProc(UINT uiID, UINT uiMsg, DWORD
                                  dwUser, DWORD dw1, DWORD dw2) {
      static volatile bool entered = false;

	W32Timer *timer=(W32Timer *)dwUser ;
	timer->OnTimerTick() ;
} ;

static void CALLBACK TimerCallbackProc(UINT uiID, UINT uiMsg, DWORD
                                  dwUser, DWORD dw1, DWORD dw2) {
      static volatile bool entered = false;

	timerCallback cb=(timerCallback)dwUser ;
	(*cb)() ;
} ;


W32Timer::W32Timer() {
	period_=-1 ;
	timer_=0 ;
	running_=false ;
} ;

W32Timer::~W32Timer() {
}

void W32Timer::SetPeriod(float msec) {
	period_=msec ;
	offset_=0 ;
} ;

bool W32Timer::Start() {
	if (period_>0) {
		offset_=period_ ;
		int newcb=int(offset_) ;
		offset_-=newcb ;
		timer_=timeSetEvent(newcb, 0, &TimerProc, (DWORD)this,TIME_ONESHOT);
		running_=true ;
	}
	return (timer_!=0) ;
} ;

void W32Timer::Stop() {
	running_=false ;
	timeKillEvent(timer_) ;
	timer_=0 ;
} ;

float W32Timer::GetPeriod() {
	return period_ ;
} ;

void W32Timer::OnTimerTick() {
	int newcb=0 ;
	if (running_) {
		SetChanged() ;
		NotifyObservers() ;
		offset_+=period_ ;
		newcb=int(offset_) ;
		offset_-=newcb ;
		assert(newcb>0) ;
		timer_=timeSetEvent(newcb, 0, &TimerProc, (DWORD)this,TIME_ONESHOT);
	}
} ;

I_Timer *W32TimerService::CreateTimer() {
	TIMECAPS c ;
	MMRESULT r=timeGetDevCaps(&c,sizeof(c)) ;
	return new W32Timer() ;
} ;

void W32TimerService::TriggerCallback(int msec,timerCallback cb) {
	timeSetEvent(msec, 0, &TimerCallbackProc, (DWORD)cb,TIME_ONESHOT) ;
} ;