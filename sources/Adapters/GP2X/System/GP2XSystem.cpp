
#include "GP2XSystem.h"
#include "Adapters/GP2X/FileSystem/GP2XFileSystem.h"
#include "Adapters/SDL/GUI/GUIFactory.h"
#include "Adapters/SDL/GUI/SDLGUIWindowImp.h"
#include "Adapters/GP2X/Audio/GP2XAudio.h"
#include "Adapters/GP2X/Midi/GP2XMidiService.h"
#include "Externals/TinyXML/tinyxml.h"
#include "Application/Model/Config.h"
#include "Application/Controllers/ControlRoom.h"
#include "Application/Player/SyncMaster.h"
#include "Application/Commands/NodeList.h"
#include "Adapters/SDL/Timer/SDLTimer.h"
#include "System/Console/Logger.h"
#include <unistd.h>
#include <fcntl.h>
#include <memory.h>
#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <sys/time.h> 
#include <malloc.h>
#include <stdlib.h>

#ifdef fopen
#undef fopen
#undef fclose
#endif

EventManager *GPSDLSystem::eventManager_ = NULL ;

// F200 Batt reading

typedef struct {
unsigned short batt;
unsigned short remocon;
} MMSP2ADC;


#define BATT_LEVEL_HIGH 0
#define BATT_LEVEL_MID 1
#define BATT_LEVEL_LOW 2
#define BATT_LEVEL_EMPTY 3


//#define DEBUG

bool GPSDLSystem::invert_=false ;
int GPSDLSystem::lastBattLevel_=100 ;
unsigned int GPSDLSystem::lastBeatCount_=0 ;

static int devbatt_;

static bool f200_ ;

int GPSDLSystem::MainLoop() {

	eventManager_->InstallMappings() ;

	// Map volume

	ControlRoom *cr=ControlRoom::GetInstance() ;

	cr->Attach(URL_VOLUME_INCREASE,"but:0:16") ;
	cr->Attach(URL_VOLUME_DECREASE,"but:0:17") ;

	return eventManager_->MainLoop() ;
} ;

void GPSDLSystem::Boot(int argc,char **argv) {

	// Install System
	System::Install(new GPSDLSystem()) ;
	System *sys=System::GetInstance() ;

	// Install FileSystem
	FileSystem::Install(new GP2XFileSystem()) ;
	Path::SetAlias("bin",".") ;
	Path::SetAlias("root",".") ;

	Config::GetInstance()->ProcessArguments(argc,argv) ;

  Path logPath("bin:lgpt.log");
  FileLogger *fileLogger=new FileLogger(logPath);
  if(fileLogger->Init().Succeeded())
  {
    Trace::GetInstance()->SetLogger(*fileLogger);    
  }
  
	// Install GUI Factory
	I_GUIWindowFactory::Install(new GUIFactory()) ;

	// Install Timers

	TimerService::GetInstance()->Install(new SDLTimerService()) ;

	// Install Sound
	AudioSettings hint ;
	Audio::Install(new GP2XAudio(hint)) ;

	// Install Midi
	MidiService::Install(new GP2XMidiService()) ;
    
	if ( SDL_Init(SDL_INIT_EVENTTHREAD | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_TIMER) < 0 )   {
		return;
	}

	SDL_EnableUNICODE(1);

	atexit(SDL_Quit);

	eventManager_=I_GUIWindowFactory::GetInstance()->GetEventManager() ;
	eventManager_->Init() ;

	bool invert=false ;
	Config *config=Config::GetInstance() ;
	const char *s=config->GetValue("INVERT") ;

	if ((s)&&(!strcmp(s,"YES"))) {
		invert=true ;
	}

	if (!invert) {
		eventManager_->MapAppButton("but:0:12",APP_BUTTON_A) ;
		eventManager_->MapAppButton("but:0:13",APP_BUTTON_B) ;
	} else {
		eventManager_->MapAppButton("but:0:13",APP_BUTTON_A) ;
		eventManager_->MapAppButton("but:0:12",APP_BUTTON_B) ;
	}
	eventManager_->MapAppButton("but:0:8",APP_BUTTON_START) ;
	eventManager_->MapAppButton("but:0:10",APP_BUTTON_L) ;
	eventManager_->MapAppButton("but:0:11",APP_BUTTON_R) ;
	eventManager_->MapAppButton("but:0:6",APP_BUTTON_RIGHT) ;
	eventManager_->MapAppButton("but:0:2",APP_BUTTON_LEFT) ;
	eventManager_->MapAppButton("but:0:4",APP_BUTTON_DOWN) ;
	eventManager_->MapAppButton("but:0:0",APP_BUTTON_UP) ;

  devbatt_ = open("/dev/batt", O_RDONLY);
  f200_=(devbatt_==-1) ;
  if (f200_) {
     devbatt_ = open("/dev/mmsp2adc", O_RDONLY);
  }	
} ;

void GPSDLSystem::Shutdown() {
  delete Audio::GetInstance() ;
  close (devbatt_);
} ;

static int      secbase; 

unsigned long GPSDLSystem::GetClock() {
   struct timeval tp;
 
   gettimeofday(&tp, NULL);  
   if (!secbase)
    {
        secbase = tp.tv_sec;
        return long(tp.tv_usec/1000.0);
     }
     return long((tp.tv_sec - secbase)*1000 + tp.tv_usec/1000.0);
}

int GPSDLSystem::GetBatteryLevel() 
{
    if (!f200_){
		unsigned int beatCount=SyncMaster::GetInstance()->GetBeatCount();
		if (beatCount!=lastBeatCount_) {
			unsigned short currentval=0;
			read (devbatt_, &currentval, 2);
	    
			if (currentval>700) currentval=700 ;
			if (currentval<650) currentval=650 ;
			lastBattLevel_= ((currentval-650)*2);
			lastBeatCount_=beatCount ;
		}
    } else {
/*        MMSP2ADC val;
        read (devbatt_, &val, sizeof(MMSP2ADC));
        switch(val.batt) {
            case BATT_LEVEL_HIGH:
                 return 100 ;
            case BATT_LEVEL_MID:
                 return 90 ;
            case BATT_LEVEL_LOW:
                 return 50 ;
            case BATT_LEVEL_EMPTY:
                 return 25 ;
        } ;
 */   }
    return lastBattLevel_ ;
} ;

void GPSDLSystem::Sleep(int millisec) {
//	if (millisec>0)
//		assert(0) ;
}

void *GPSDLSystem::Malloc(unsigned size) {
    void *ptr=malloc(size) ;
 //   Trace::Debug("m(%d):0x%x",size,ptr) ;
	return ptr ;
}

void GPSDLSystem::Free(void *ptr) {
//    Trace::Debug("fr:0x%x",ptr);
	free(ptr) ;
//	Trace::Debug("~fr") ;
} 

extern "C" void *gpmemset(void *s1,unsigned char val,int n) ;

void GPSDLSystem::Memset(void *addr,char val,int size) {
	gpmemset(addr,val,size) ;
} ;


extern "C" void *gpmemcpy(void *s1, const void *s2, int n) ;

void *GPSDLSystem::Memcpy(void *s1, const void *s2, int n) {
        return gpmemcpy(s1,s2,n);
}

void GPSDLSystem::PostQuitMessage() {
     eventManager_->PostQuitMessage() ;
}

unsigned int GPSDLSystem::GetMemoryUsage() {
	struct mallinfo m=mallinfo();	
	return m.uordblks ;
}
