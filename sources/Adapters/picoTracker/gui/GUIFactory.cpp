#include "GUIFactory.h"
#include "picoTrackerEventManager.h"
#include "picoTrackerGUIWindowImp.h"

GUIFactory::GUIFactory(){};

I_GUIWindowImp &GUIFactory::CreateWindowImp(GUICreateWindowParams &p) {
  return *(new picoTrackerGUIWindowImp(p));
}

EventManager *GUIFactory::GetEventManager() {
  return picoTrackerEventManager::GetInstance();
}
