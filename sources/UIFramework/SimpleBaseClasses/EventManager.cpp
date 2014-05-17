/*
 *  EventManager.cpp
 *  lgpt
 *
 *  Created by Marc Nostromo on 23/03/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "EventManager.h"
#include "Application/Model/Config.h"
#include "Application/Controllers/ControlRoom.h"
#include "Application/Commands/NodeList.h"

bool EventManager::Init() {
	return true ;
}

void EventManager::MapAppButton(const char *mapping,AppButton button) {
	
	std::string mapString ;
	if (!strchr(mapping,':')) {
		mapString="key:0:" ;
	} ;
	mapString+=mapping ;
	mapping_[button]=mapString ;
} ;

void EventManager::mapConfigKey(AppButton button,const char *keyname) {
	
	Config *config=Config::GetInstance() ;
	
	// Read the configuration file and look if we got a definition 
	const char *key=config->GetValue(keyname) ;
	if (key) {
		MapAppButton(key,button) ;
	}
}

void EventManager::InstallMappings() {
	// Read config file for key assignment
	
	Trace::Debug("Mapping config") ;
	
	mapConfigKey(APP_BUTTON_A,"KEY_A") ;
	mapConfigKey(APP_BUTTON_B,"KEY_B") ;
	mapConfigKey(APP_BUTTON_LEFT,"KEY_LEFT") ;
	mapConfigKey(APP_BUTTON_RIGHT,"KEY_RIGHT") ;
	mapConfigKey(APP_BUTTON_UP,"KEY_UP") ;
	mapConfigKey(APP_BUTTON_DOWN,"KEY_DOWN") ;
	mapConfigKey(APP_BUTTON_L,"KEY_LSHOULDER") ;
	mapConfigKey(APP_BUTTON_R,"KEY_RSHOULDER") ;
	mapConfigKey(APP_BUTTON_START,"KEY_START") ;
	mapConfigKey(APP_BUTTON_VOLINC,"KEY_VOLINC") ;
	mapConfigKey(APP_BUTTON_VOLDEC,"KEY_VOLDEC") ;
	
	ControlRoom *cr=ControlRoom::GetInstance() ;
	
	cr->Attach(URL_EVENT_A,mapping_[APP_BUTTON_A].c_str()) ;
	cr->Attach(URL_EVENT_B,mapping_[APP_BUTTON_B].c_str()) ;
	cr->Attach(URL_EVENT_LEFT,mapping_[APP_BUTTON_LEFT].c_str()) ;
	cr->Attach(URL_EVENT_RIGHT,mapping_[APP_BUTTON_RIGHT].c_str()) ;
	cr->Attach(URL_EVENT_UP,mapping_[APP_BUTTON_UP].c_str()) ;
	cr->Attach(URL_EVENT_DOWN,mapping_[APP_BUTTON_DOWN].c_str()) ;
	cr->Attach(URL_EVENT_LSHOULDER,mapping_[APP_BUTTON_L].c_str()) ;
	cr->Attach(URL_EVENT_RSHOULDER,mapping_[APP_BUTTON_R].c_str()) ;
	cr->Attach(URL_EVENT_START,mapping_[APP_BUTTON_START].c_str()) ;
	cr->Attach(URL_VOLUME_INCREASE,mapping_[APP_BUTTON_VOLINC].c_str()) ;
	cr->Attach(URL_VOLUME_DECREASE,mapping_[APP_BUTTON_VOLDEC].c_str()) ;
	
}