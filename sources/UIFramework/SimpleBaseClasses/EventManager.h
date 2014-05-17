/*
 *  EventManager.h
 *  lgpt
 *
 *  Created by Marc Nostromo on 23/03/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once 

#include <string>

enum AppButton {
	APP_BUTTON_A = 0,
	APP_BUTTON_B ,
	APP_BUTTON_LEFT,
	APP_BUTTON_RIGHT,
	APP_BUTTON_UP,
	APP_BUTTON_DOWN,
	APP_BUTTON_L,
	APP_BUTTON_R,
	APP_BUTTON_START,
	APP_BUTTON_VOLINC,
	APP_BUTTON_VOLDEC,
	APP_BUTTON_LAST
};

class EventManager {
public:
	EventManager() {} ;
	virtual ~EventManager() {} ;
	virtual bool Init() ;
	virtual int MainLoop() =0 ;
	virtual void PostQuitMessage() =0 ;
	virtual int GetKeyCode(const char *name) =0 ;
	void MapAppButton(const char *mapping,AppButton button) ;
	void InstallMappings() ;
protected:
	void mapConfigKey(AppButton button,const char *keyName) ;
private:
	std::string mapping_[APP_BUTTON_LAST] ;
} ;
