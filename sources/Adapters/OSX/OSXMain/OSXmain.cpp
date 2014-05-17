#include "Application/Application.h"
#include "Adapters/OSX/OSXSystem/OSXSystem.h"
#include "Adapters/SDL/GUI/SDLGUIWindowImp.h"

#include <string.h>

int main(int argc,char *argv[]) 
{
	OSXSystem::Boot(argc,argv) ;

	SDLCreateWindowParams params ;
	params.title="littlegptracker" ;
	params.cacheFonts_=false ;

	Application::GetInstance()->Init(params) ;

	int retval=OSXSystem::MainLoop() ;

	return retval ;
}
