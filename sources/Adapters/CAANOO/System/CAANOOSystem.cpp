
#include "CAANOOSystem.h"
#include "Adapters/Unix/FileSystem/UnixFileSystem.h"
#include "Adapters/SDL/GUI/GUIFactory.h"
#include "Adapters/SDL/GUI/SDLGUIWindowImp.h"
#include "Adapters/SDL/GUI/SDLEventManager.h"
#include "Adapters/CAANOO/Audio/CAANOOAudio.h"
#include "Adapters/CAANOO/Midi/CAANOOMidiService.h"
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
EventManager *GPSDLSystem::eventManager_ = NULL ;
bool GPSDLSystem::invert_=false ;
int GPSDLSystem::lastBattLevel_=100 ;
unsigned int GPSDLSystem::lastBeatCount_=0 ;

static int devbatt_;

static bool f200_ ;

int GPSDLSystem::MainLoop() 
{
	eventManager_->InstallMappings() ;
	return eventManager_->MainLoop() ;
} ;

void GPSDLSystem::Boot(int argc,char **argv) {

	// Install System
	System::Install(new GPSDLSystem()) ;
	System *sys=System::GetInstance() ;

	// Install FileSystem
	FileSystem::Install(new UnixFileSystem()) ;
	Path::SetAlias("bin",".") ;
	Path::SetAlias("root",".") ;

	Config::GetInstance()->ProcessArguments(argc,argv) ;

#ifdef _DEBUG
  Trace::GetInstance()->SetLogger(*(new StdOutLogger()));
#else
  Path logPath("bin:lgpt.log");
  FileLogger *fileLogger=new FileLogger(logPath);
  if(fileLogger->Init().Succeeded())
  {
    Trace::GetInstance()->SetLogger(*fileLogger);    
  }
#endif

	// Install GUI Factory
	I_GUIWindowFactory::Install(new GUIFactory()) ;

	// Install Timers

	TimerService::GetInstance()->Install(new SDLTimerService()) ;

	// Install Sound
	AudioSettings hints ;
        hints.bufferSize_= 256 ;
        hints.preBufferCount_=2 ;

	Audio::Install(new CAANOOAudio(hints)) ;

	// Install Midi
	MidiService::Install(new CAANOOMidiService()) ;
    
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
		eventManager_->MapAppButton("but:0:0",APP_BUTTON_A) ;
		eventManager_->MapAppButton("but:0:3",APP_BUTTON_B) ;
	} else {
		eventManager_->MapAppButton("but:0:3",APP_BUTTON_A) ;
		eventManager_->MapAppButton("but:0:0",APP_BUTTON_B) ;
	}
	eventManager_->MapAppButton("but:0:6",APP_BUTTON_START) ;
	eventManager_->MapAppButton("but:0:4",APP_BUTTON_L) ;
	eventManager_->MapAppButton("but:0:5",APP_BUTTON_R) ;
	eventManager_->MapAppButton("joy:0:0+",APP_BUTTON_RIGHT) ;
	eventManager_->MapAppButton("joy:0:0-",APP_BUTTON_LEFT) ;
	eventManager_->MapAppButton("joy:0:1+",APP_BUTTON_DOWN) ;
	eventManager_->MapAppButton("joy:0:1-",APP_BUTTON_UP) ;

  devbatt_ = open("/dev/batt", O_RDONLY);
  f200_=(devbatt_==-1) ;
  if (f200_)
  {
    devbatt_ = open("/dev/mmsp2adc", O_RDONLY);
  }	
}

void GPSDLSystem::Shutdown() 
{
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

int GPSDLSystem::GetBatteryLevel() {

	
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

void GPSDLSystem::Sleep(int millisec) 
{
}

void *GPSDLSystem::Malloc(unsigned size) 
{
  void *ptr=malloc(size) ;
	return ptr ;
}

void GPSDLSystem::Free(void *ptr)
{
	free(ptr) ;
} 

extern "C" void *gpmemset(void *s1,unsigned char val,int n) ;

void GPSDLSystem::Memset(void *addr,char val,int size) 
{
	gpmemset(addr,val,size) ;
} ;


extern "C" void *gpmemcpy(void *s1, const void *s2, int n) ;

void *GPSDLSystem::Memcpy(void *s1, const void *s2, int n) 
{
        return gpmemcpy(s1,s2,n);
}

void GPSDLSystem::PostQuitMessage() {
     SDLEventManager::GetInstance()->PostQuitMessage() ;
}

unsigned int GPSDLSystem::GetMemoryUsage() {
	struct mallinfo m=mallinfo();	
	return m.uordblks ;
}
