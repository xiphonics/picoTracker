#include "GUIFactory.h"
#include "picoTrackerEventManager.h"
#include "picoTrackerGUIWindowImp.h"

GUIFactory::GUIFactory(){};

I_GUIWindowImp &GUIFactory::CreateWindowImp(GUICreateWindowParams &p) {
  static char guiImpMemBuf[sizeof(picoTrackerGUIWindowImp)];
  return *(new (guiImpMemBuf) picoTrackerGUIWindowImp(p));
}

EventManager *GUIFactory::GetEventManager() {
  return picoTrackerEventManager::GetInstance();
}
