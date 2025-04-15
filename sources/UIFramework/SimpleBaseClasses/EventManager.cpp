/*
 *  EventManager.cpp
 *  lgpt
 *
 *  Created by Marc Nostromo on 23/03/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "EventManager.h"
#include "Application/Commands/NodeList.h"
#include "Application/Controllers/ControlRoom.h"
#include "Application/Model/Config.h"
#include <cstring>

bool EventManager::Init() { return true; }

void EventManager::MapAppButton(const char *mapping, AppButton button) {

  std::string mapString;
  if (!strchr(mapping, ':')) {
    mapString = "key:0:";
  };
  mapString += mapping;
  mapping_[button] = mapString;
};

void EventManager::InstallMappings() {
  // Read config file for key assignment

  Trace::Debug("Mapping config");
  ControlRoom *cr = ControlRoom::GetInstance();

  cr->Attach(URL_EVENT_PLAY, mapping_[APP_BUTTON_PLAY].c_str());
  cr->Attach(URL_VOLUME_INCREASE, mapping_[APP_BUTTON_VOLINC].c_str());
  cr->Attach(URL_VOLUME_DECREASE, mapping_[APP_BUTTON_VOLDEC].c_str());
}
