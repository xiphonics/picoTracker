
#include "WSDLSystem.h"
#include "Adapters/RTAudio/RTAudioStub.h"
#include "Adapters/SDL/GUI/SDLEventManager.h"
#include "Adapters/SDL/GUI/GUIFactory.h"
#include "Adapters/SDL/GUI/SDLGUIWindowImp.h"
#include "Adapters/RTAudio/RTAudioStub.h"
#include "Adapters/RTMidi/RTMidiService.h"
#include "Adapters/W32/Midi/W32MidiService.h"
#include "Adapters/W32/Audio/W32Audio.h"
#include "Adapters/W32FileSystem/W32FileSystem.h"
#include "Adapters/W32/Process/W32Process.h"
#include "Adapters/W32/Timer/W32Timer.h"
#include "Application/Model/Config.h"
#include "System/Console/Logger.h"
#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>

EventManager *WSDLSystem::eventManager_ = NULL ;

int WSDLSystem::MainLoop() {
	eventManager_->InstallMappings() ;
	return eventManager_->MainLoop() ;
} ;

void WSDLSystem::Boot(int argc,char **argv) {

	SDL_putenv("SDL_VIDEODRIVER=directx") ;

	// Install System
	System::Install(new WSDLSystem()) ;

	// Install FileSystem
	FileSystem::Install(new W32FileSystem()) ;
  // Setup logger
  Trace::GetInstance()->SetLogger(*(new StdOutLogger()));

	// Get application directory & install platform specific aliases

	HMODULE module = GetModuleHandle(NULL);
	char temp_path[MAX_PATH];
	int length = GetModuleFileName(module,temp_path,MAX_PATH);
	int n = (int)strlen(temp_path)-1;
	while (temp_path[n] !='\\')
		n--;
	if (n<3)
		n=3;

	temp_path[n]=0;

	Path::SetAlias("bin",temp_path) ;

	Path::SetAlias("root","bin:..") ;

  // Tracing
  
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
  
	Config *config=Config::GetInstance() ;
	config->ProcessArguments(argc,argv) ;

	// Install GUI Factory
	I_GUIWindowFactory::Install(new GUIFactory()) ;

	// Install Timers

	TimerService::GetInstance()->Install(new W32TimerService()) ;

//	(new RTAudioStub(config->GetValue("AUDIODRIVER")))->Init() ;

	// Allow to use either Direct Sound of MMSYSTEM

	AudioSettings hints ;
	const char *api=config->GetValue("AUDIOAPI") ;
	Audio *audio=0 ;

	if (api&&(!_stricmp(api,"MMSYSTEM"))) {
		hints.audioAPI_="MMSYSTEM" ;
		hints.audioDevice_="" ;
		hints.bufferSize_=512 ;
		hints.preBufferCount_=10;
		audio=new W32Audio(hints) ;
	} else {
		hints.audioAPI_="DSound" ;
		hints.audioDevice_="" ;
		hints.bufferSize_=512 ;
		hints.preBufferCount_=10;
		audio=new RTAudioStub(hints) ;
	}

	Audio::Install(audio) ;

	// Install Midi
	MidiService::Install(new RTMidiService()) ;

	// Install Threads

	SysProcessFactory::Install(new W32ProcessFactory()) ;

	if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK|SDL_INIT_TIMER) < 0 )   {
		return;
	}
	SDL_EnableUNICODE(1);
	SDL_ShowCursor(SDL_DISABLE);

	atexit(SDL_Quit);

	eventManager_=I_GUIWindowFactory::GetInstance()->GetEventManager() ;
	eventManager_->Init() ;

	eventManager_->MapAppButton("a",APP_BUTTON_A) ;
	eventManager_->MapAppButton("s",APP_BUTTON_B) ;
	eventManager_->MapAppButton("left",APP_BUTTON_LEFT) ;
	eventManager_->MapAppButton("right",APP_BUTTON_RIGHT) ;
	eventManager_->MapAppButton("up",APP_BUTTON_UP) ;
	eventManager_->MapAppButton("down",APP_BUTTON_DOWN) ;
	eventManager_->MapAppButton("right ctrl",APP_BUTTON_L) ;
	eventManager_->MapAppButton("left ctrl",APP_BUTTON_R) ;
	eventManager_->MapAppButton("space",APP_BUTTON_START) ;

} ;

void WSDLSystem::Shutdown() {
	delete Audio::GetInstance() ;
} ;

//void WSDLSystem::readConfig() {
//	SDLInput::GetInstance()->ReadConfig() ;
//} ;

unsigned long WSDLSystem::GetClock() {
	return (clock()*1000)/CLOCKS_PER_SEC ;
}

void WSDLSystem::Sleep(int millisec) {
	if (millisec>0)
		::Sleep(millisec) ;
}

void *WSDLSystem::Malloc(unsigned size) {
	return malloc(size) ;
}

void WSDLSystem::Free(void *ptr) {
	free(ptr) ;
} 

void WSDLSystem::Memset(void *addr,char val,int size) {
    
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

void *WSDLSystem::Memcpy(void *s1, const void *s2, int n) 
{
    return memcpy(s1,s2,n) ;
} ;  

void WSDLSystem::PostQuitMessage() 
{
	SDLEventManager::GetInstance()->PostQuitMessage()  ;
} ; 

unsigned int  WSDLSystem::GetMemoryUsage() 
{
	return 0 ;
} ;


std::string WSDLSystem::SGetLastErrorString()
{
    LPVOID lpMsgBuf;

    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    std::string error = (char *)lpMsgBuf;

    LocalFree(lpMsgBuf);

    return error;
}