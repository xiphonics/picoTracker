#include "GUIFactory.h"
#include "advEventManager.h"
#include "advGUIWindowImp.h"

GUIFactory::GUIFactory(){};

I_GUIWindowImp &GUIFactory::CreateWindowImp(GUICreateWindowParams &p) {
  static char guiImpMemBuf[sizeof(advGUIWindowImp)];
  return *(new (guiImpMemBuf) advGUIWindowImp(p));
}

EventManager *GUIFactory::GetEventManager() {
  return advEventManager::GetInstance();
}
