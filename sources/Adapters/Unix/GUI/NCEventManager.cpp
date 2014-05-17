/*
 *  NCEventManager.cpp
 *  lgpt
 *
 *  Created by Marc Nostromo on 24/03/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "NCEventManager.h"
#include "NCGUIWindowImp.h"
#include "Application/Application.h"

#include <ncurses.h>

NCEventManager::NCEventManager() {
}

NCEventManager::~NCEventManager() {
	
}

bool NCEventManager::Init() {
	return true ;
}

int  NCEventManager::MainLoop() {
	GUIWindow *appWindow=Application::GetInstance()->GetWindow() ;
	NCGUIWindowImp *ncWindow=(NCGUIWindowImp *)appWindow->GetImpWindow() ;

	ncWindow->ProcessExpose() ;
	bool finished = false;
	while (!finished) {
		char ch = getch();
		if (ch == 'q')
		{
			finished = true;
		}
	} 
}

void  NCEventManager::PostQuitMessage() {
	NYI ;
}

int  NCEventManager::GetKeyCode(const char *name) {
	NYI ;
}
