#include "Application/Application.h"
#include "Adapters/DINGOO/System/DINGOOSystem.h"
#include "Foundation/T_Singleton.h"
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
}



