#include "NDSSystem.h"
#include "Adapters/NDS/UI/GUIFactory.h"
#include "Adapters/NDS/UI/NDSGUIWindowImp.h"
#include "Adapters/NDS/Sound/NDSSound.h"
#include "Adapters/Dummy/Midi/DummyMidi.h"
#include "Adapters/NDS/FileSystem/NDSFileSystem.h"
#include "System/io/Trace.h"
#include "NDSEventQueue.h"
#include "Adapters/NDS/UI/NDSGUIWindowImp.h"
#include "nds.h"

bool NDSSystem::finished_=false ;
bool NDSSystem::redrawing_=false ;
bool NDSSystem::escPressed_=false ;
u16 NDSSystem::buttonMask_=0 ;

bool NDSSystem::isRepeating_=false ;
unsigned long NDSSystem::time_=0 ;
unsigned int NDSSystem::keyRepeat_=25 ;
unsigned int NDSSystem::keyDelay_=500 ;
unsigned int NDSSystem::keyKill_=5 ;

u16 gTime_=0 ;

void timerHandler() {
     gTime_++ ;
} 

void NDSSystem::Boot() {

	Trace::Dump("Booting..\n") ;

	// Install System
	System::Install(new NDSSystem()) ;

	// Install GUI Factory
	I_GUIWindowFactory::Install(new GUIFactory()) ;

	// Install FileSystem
	FileSystem::Install(new NDSFileSystem()) ;

	// Install Sound
	Sound::Install(new NDSSound()) ;

	// Install Midi
	Midi::Install(new DummyMidi()) ;

    // Install timer interrupt
    
    u16 freq = 1000;
    TIMER0_DATA = TIMER_FREQ(freq);
    TIMER0_CR = TIMER_ENABLE | TIMER_DIV_1 | TIMER_IRQ_REQ;
    irqSet(IRQ_TIMER0, timerHandler);
    irqEnable(IRQ_TIMER0);    
    
} ;

void NDSSystem::Shutdown() {
} ;


int NDSSystem::MainLoop() {

    finished_=false ;
	// Main message loop:
	while (!finished_) 
	{
       ProcessOneEvent() ;
       NDSEventQueue *queue=NDSEventQueue::GetInstance() ;
       NDSEvent *event=queue->Pop(true) ;
       if (event) {
          redrawing_=true ;
    //	  Trace::Debug("Re") ;
          NDSGUIWindowImp::ProcessEvent(*event) ;
          delete event ;
          queue->Empty() ; // Avoid duplicates redraw
          redrawing_=false ;
    //	  Trace::Debug("~Re") ;
       }          
	}
    return 0 ;
}

void NDSSystem::ProcessOneEvent() {

  u16 newMask,sendMask ;
   
   
  if (redrawing_) return ;
 
    bool gotEvent=false ;
	// Get current mask

    scanKeys() ;
    
	newMask= keysHeld() ;

	if ((newMask&KEY_SELECT)&&(newMask&KEY_L)&&(newMask&KEY_R)) {
       if (!escPressed_) {
           NDSEvent event ;
           event.type_=NDSET_ESC ;
           NDSGUIWindowImp::ProcessEvent(event) ;
           escPressed_=true ;
       }                                                                 
	} else {
       escPressed_=false ;
    } ;
   
    // compute mask to send ;

    sendMask=(newMask^buttonMask_)|(newMask&(KEY_LEFT|KEY_RIGHT|KEY_UP|KEY_DOWN)) ;
    unsigned long now=gTime_;
    // see if we're repeating
    if (newMask==buttonMask_) {
        if ((isRepeating_)&& ((now-time_)>keyRepeat_)) {
            gotEvent=(sendMask!=0) ;
        }
        if ((!isRepeating_)&& ((now-time_)>keyDelay_)) {
             gotEvent=(sendMask!=0) ;
             if (gotEvent) isRepeating_=true ;
        }
    } else {
        if ((now-time_)>keyKill_) {
            gotEvent=(sendMask!=0) ;
            if (gotEvent) isRepeating_=false ;
        }
    }
    if (gotEvent) {
            time_=gTime_; // Get time here so delay is
                   // independant of processing speed 
                   
//                Trace::Debug("Pe") ;
            NDSGUIWindowImp::ProcessButtonChange(sendMask,newMask) ;
            buttonMask_=newMask ;
//            Trace::Debug("%d: mask=%x",gTime_,sendMask) ;
//                Trace::Debug("~Pe") ;
    }

} ;

int NDSSystem::GetBatteryLevel() {
    return 100 ;
} ;

void NDSSystem::AddUserLog(const char *msg) {
     printf("%s\n",msg) ;
} ;

void NDSSystem::PostQuitMessage() {
     finished_=true ;
}

unsigned long NDSSystem::GetClock() {
	return gTime_ ;
}

void *NDSSystem::Malloc(unsigned size) {
	return malloc(size) ;
}

void NDSSystem::Free(void *ptr) {
	free(ptr) ;
} 

void NDSSystem::Memset(void *addr,char val,int size) {
     memset(addr,val,size) ;
} ;

void *NDSSystem::Memcpy(void *s1, const void *s2, int n) {
    return memcpy(s1,s2,n) ;
} ;  
