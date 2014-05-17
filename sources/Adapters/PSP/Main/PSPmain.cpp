// VC6GUI.cpp : Defines the entry point for the application.
//

#include "Application/Application.h"
#include "Adapters/PSP/System/PSPSystem.h"
#include "Foundation/T_Singleton.h"
#include <SDL/SDL.h>
#include <string.h>
#include "Adapters/SDL/GUI/SDLGUIWindowImp.h"
#include "Application/Persistency/PersistencyService.h" 
#include "Adapters/SDL/GUI/SDLGUIWindowImp.h"

#include <pspkernel.h>
#include <pspdebug.h>

/* Define the module info section */
PSP_MODULE_INFO("AUDIOLIBDEMO", 0, 1, 1);
/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
/* Define printf, just to make typing easier */

/* Define printf, just to make typing easier */
#define printf	pspDebugScreenPrintf

/* 
   This part of the code is more or less identical to the sdktest sample 
*/

/* Define printf, just to make typing easier */
#define printf	pspDebugScreenPrintf

/* Exit callback */
int exitCallback(int arg1, int arg2, void *common) {
	sceKernelExitGame();
	return 0;
}

/* Callback thread */
int callbackThread(SceSize args, void *argp) {
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", exitCallback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();

	return 0;
}

/* Sets up the callback thread and returns its thread id */
int setupCallbacks(void) {
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", callbackThread, 0x11, 0xFA0, 0, 0);
	if (thid >= 0) {
		sceKernelStartThread(thid, 0, 0);
	}
	return thid;
}

int main(int argc,char *argv[]) 
{

	setupCallbacks();

	PSPSystem::Boot(argc,argv) ;

	SDLCreateWindowParams params ;
	params.title="littlegptracker" ;
	params.cacheFonts_=false ;
    params.framebuffer_=false ;
	Application::GetInstance()->Init(params) ;
	PSPSystem::MainLoop() ;
    PSPSystem::Shutdown() ; 
	sceKernelExitGame();
	return 0 ;
}

