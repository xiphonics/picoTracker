#include "Application/Application.h"
#include "Adapters/DEB/System/DEBSystem.h"
#include <string.h>
#include "Adapters/SDL/GUI/SDLGUIWindowImp.h"

int main(int argc,char *argv[]) 
{
	DEBSystem::Boot(argc,argv) ;

	SDLCreateWindowParams params ;
	params.title="littlegptracker" ;
	params.cacheFonts_=true ;

	Application::GetInstance()->Init(params) ;

	int retval=DEBSystem::MainLoop() ;

	return retval ;
}



void _assert() {
} ;
