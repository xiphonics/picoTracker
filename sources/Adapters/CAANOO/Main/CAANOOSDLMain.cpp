// VC6GUI.cpp : Defines the entry point for the application.
//

#include "Application/Application.h"
#include "Adapters/CAANOO/System/CAANOOSystem.h"
#include <SDL/SDL.h>
#include <string.h>
#include "Adapters/SDL/GUI/SDLGUIWindowImp.h"
#include "Application/Persistency/PersistencyService.h" 
#include "Adapters/SDL/GUI/SDLGUIWindowImp.h"

int main(int argc,char *argv[]) 
{
	GPSDLSystem::Boot(argc,argv) ;

	SDLCreateWindowParams params ;
	params.title="littlegptracker" ;
	params.cacheFonts_=true ;
  params.framebuffer_=true ;

	Application::GetInstance()->Init(params) ;

	GPSDLSystem::MainLoop() ;
  GPSDLSystem::Shutdown() ;
	return 0 ;
}



