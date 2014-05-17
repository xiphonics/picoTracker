
#include "PSPSystem.h"
#include "Adapters/Dummy/Midi/DummyMidi.h"
#include "Adapters/SDL/Audio/SDLAudio.h"
#include "Adapters/SDL/GUI/SDLEventManager.h"
#include "Adapters/SDL/GUI/GUIFactory.h"
#include "Adapters/SDL/GUI/SDLGUIWindowImp.h"
#include "Adapters/SDL/Process/SDLProcess.h"
#include "Adapters/PSP/FileSystem/PSPFileSystem.h"
#include "Adapters/SDL/Timer/SDLTimer.h"
#include "Application/Model/Config.h"
#include "System/Console/Logger.h"
#include <time.h>
#include <pspdebug.h>
#include <sys/time.h>
#include <malloc.h>
#include <stdlib.h>

EventManager *PSPSystem::eventManager_ = NULL ;

int PSPSystem::MainLoop() 
{
	eventManager_->InstallMappings() ;
	return eventManager_->MainLoop() ;
} ;

void PSPSystem::Boot(int argc,char **argv) {
#ifdef PSPDEBUG
	pspDebugScreenInit();
#endif

	// Install System
	System::Install(new PSPSystem()) ;

	// Install FileSystem
	FileSystem::Install(new PSPFileSystem()) ;

	Path bootPath(argv[0]) ;
	Path parent=bootPath.GetParent() ;

	Path::SetAlias("bin",parent.GetPath().c_str()) ;
	Path::SetAlias("root",parent.GetPath().c_str()) ;

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

	AudioSettings hints ;
	hints.bufferSize_=128 ;
	hints.preBufferCount_=6 ;
	Audio::Install(new SDLAudio(hints)) ;

	// Install Midi
	MidiService::Install(new DummyMidi()) ;

	// Install Threads

	SysProcessFactory::Install(new SDLProcessFactory()) ;

	if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK|SDL_INIT_TIMER) < 0 )   {
		return;
	}
	SDL_EnableUNICODE(1);
	SDL_ShowCursor(SDL_DISABLE);

	atexit(SDL_Quit);

 	eventManager_=I_GUIWindowFactory::GetInstance()->GetEventManager() ;
	eventManager_->Init() ;

	// PSP SDL Basic config

	bool invert=false ;
	Config *config=Config::GetInstance() ;
	const char *s=config->GetValue("INVERT") ;

	if ((s)&&(!strcmp(s,"YES"))) {
		invert=true ;
	}

	if (!invert) {
		eventManager_->MapAppButton("but:0:0",APP_BUTTON_B) ;
		eventManager_->MapAppButton("but:0:3",APP_BUTTON_A) ;
	}else {
		eventManager_->MapAppButton("but:0:0",APP_BUTTON_A) ;
		eventManager_->MapAppButton("but:0:3",APP_BUTTON_B) ;
	}
	eventManager_->MapAppButton("but:0:7",APP_BUTTON_LEFT) ;
	eventManager_->MapAppButton("but:0:9",APP_BUTTON_RIGHT) ;
	eventManager_->MapAppButton("but:0:8",APP_BUTTON_UP) ;
	eventManager_->MapAppButton("but:0:6",APP_BUTTON_DOWN) ;
	eventManager_->MapAppButton("but:0:4",APP_BUTTON_L) ;
	eventManager_->MapAppButton("but:0:5",APP_BUTTON_R) ;
	eventManager_->MapAppButton("but:0:11",APP_BUTTON_START) ;

} ;

void PSPSystem::Shutdown() {
} ;

unsigned long PSPSystem::GetClock() {
	struct timeval now;
	Uint32 ticks;
	gettimeofday(&now, NULL);
	ticks=(now.tv_sec)*1000+(now.tv_usec)/1000;
	return(ticks);
}

void PSPSystem::Sleep(int millisec) {
/*	if (millisec>0)
		::Sleep(millisec) ;
*/}

void *PSPSystem::Malloc(unsigned size) {
	return malloc(size) ;
}

void PSPSystem::Free(void *ptr) {
	free(ptr) ;
} 

void PSPSystem::Memset(void *addr,char val,int size) {
    
    unsigned int ad=(unsigned int)addr ;
    if (((ad&0x3)==0)&&((size&0x3)==0)) { // Are we 4-byte aligned ?
        unsigned int intVal=0 ;
        for (int i=0;i<4;i++) {
             intVal=(intVal<<8)+val ;  
        }
        unsigned int *dst=(unsigned int *)addr ;
        size_t intSize=size>>2 ;
        
        for (unsigned int i=0;i<intSize;i++) {
            *dst++=intVal ;
        }        
    } else {
        memset(addr,val,size) ;
    } ;
} ;

void *PSPSystem::Memcpy(void *s1, const void *s2, int n) {
    return memcpy(s1,s2,n) ;
} ;  
/*
void PSPSystem::AddUserLog(const char *msg) {
#ifdef PSPDEBUG
	pspDebugScreenPrintf("%s\n",msg) ;
#endif
};
*/
void PSPSystem::PostQuitMessage() {
	SDLEventManager::GetInstance()->PostQuitMessage() ;
} ; 

unsigned int PSPSystem::GetMemoryUsage() {
	struct mallinfo m=mallinfo();	
	return m.uordblks ;
}