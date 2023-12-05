#include "GUIFactory.h"
#include "SDLEventManager.h"
#include "SDLGUIWindowImp.h"
#ifndef _USE_NCURSES_

GUIFactory::GUIFactory(){};

I_GUIWindowImp &GUIFactory::CreateWindowImp(GUICreateWindowParams &p) {
  return *(new SDLGUIWindowImp(p));
}

EventManager *GUIFactory::GetEventManager() {
  return SDLEventManager::GetInstance();
}
#endif