#include "EventDispatcher.h"
#include "System/Console/Trace.h"
#include "Application/Model/Config.h"
#include "SDL/SDL.h"

int EventDispatcher::keyRepeat_=30 ;
int EventDispatcher::keyDelay_=500 ;

Uint32 OnTimer(Uint32 interval) {
	return EventDispatcher::GetInstance()->OnTimerTick() ;
} ;

EventDispatcher::EventDispatcher() {
	window_=0;
	eventMask_=0 ;

	// Read config file key repeat

	Config *config=Config::GetInstance() ;
	const char *s=config->GetValue("KEYDELAY") ;
	if (s) {
		keyDelay_=atoi(s) ;
	}

	s=config->GetValue("KEYREPEAT") ;
	if (s) {
		keyRepeat_=atoi(s) ;
	}

	repeatMask_=0 ;
	repeatMask_|=(1<<EPBT_LEFT) ;
	repeatMask_|=(1<<EPBT_RIGHT) ;
	repeatMask_|=(1<<EPBT_UP) ;
	repeatMask_|=(1<<EPBT_DOWN) ;

	timer_=TimerService::GetInstance()->CreateTimer() ;
	timer_->AddObserver(*this) ;

} ;

EventDispatcher::~EventDispatcher() {
	timer_->RemoveObserver(*this) ;
	SAFE_DELETE(timer_) ;
} ;

void EventDispatcher::Execute(FourCC id,float value) {

	if (window_) {
		GUIEventPadButtonType mapping ;
		switch(id) {
			case TRIG_EVENT_A:
				mapping=EPBT_A;
				break ;
			case TRIG_EVENT_B:
				mapping=EPBT_B;
				break ;
			case TRIG_EVENT_LEFT:
				mapping=EPBT_LEFT;
				break ;
			case TRIG_EVENT_RIGHT:
				mapping=EPBT_RIGHT;
				break ;
			case TRIG_EVENT_UP:
				mapping=EPBT_UP;
				break ;
			case TRIG_EVENT_DOWN:
				mapping=EPBT_DOWN;
				break ;
			case TRIG_EVENT_LSHOULDER:
				mapping=EPBT_L;
				break ;
			case TRIG_EVENT_RSHOULDER:
				mapping=EPBT_R;
				break ;
			case TRIG_EVENT_START:
				mapping=EPBT_START;
				break ;
				//	EPBT_SELECT
		}

		// Compute mask and repeat if needed

		if (value>0.5) {
			eventMask_|=(1<<mapping) ;
		} else {
			eventMask_^=(1<<mapping) ;
		}

		// Dispatch event to window

		unsigned long now=System::GetInstance()->GetClock();
		GUIEventType type=(value>0.5)? ET_PADBUTTONDOWN:ET_PADBUTTONUP ;
		GUIEvent event(mapping,type,now,0,0,0) ;
		window_->DispatchEvent(event) ;


		if (eventMask_&repeatMask_) {
			timer_->SetPeriod(float(keyDelay_)) ;
			timer_->Start() ;
		} else {
			timer_->Stop() ;
		}
	} ;
};

void EventDispatcher::SetWindow(GUIWindow *window) {
	window_=window ;
} ;

unsigned int EventDispatcher::OnTimerTick() {

	unsigned sendMask=(eventMask_&repeatMask_) ;
	unsigned long now=System::GetInstance()->GetClock();

	if (sendMask) {
		int current=0 ;
		while (sendMask) {
			if (sendMask&1) {
				GUIEvent event(current,ET_PADBUTTONDOWN,now,0,0,0) ;
				window_->DispatchEvent(event) ;
			}
			sendMask>>=1 ;
			current++ ;
		}		
		return keyRepeat_ ;
	}
	return 0 ;
} ;

void EventDispatcher::Update(Observable &o,I_ObservableData *d) {
	unsigned int tick=OnTimerTick() ;
	if (tick) {
		timer_->SetPeriod(float(tick)) ;
	} else {
		timer_->Stop() ;
	};
} ;
