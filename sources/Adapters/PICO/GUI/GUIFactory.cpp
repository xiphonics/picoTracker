#include "GUIFactory.h"
#include "HWEventManager.h"
#include "HWGUIWindowImp.h"

GUIFactory::GUIFactory(){};

I_GUIWindowImp &GUIFactory::CreateWindowImp(GUICreateWindowParams &p) {
  return *(new HWGUIWindowImp(p));
}

EventManager *GUIFactory::GetEventManager() {
  return HWEventManager::GetInstance();
}
