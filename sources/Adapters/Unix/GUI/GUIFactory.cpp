#include "GUIFactory.h"
#include "NCGUIWindowImp.h"
#include "NCEventManager.h"

#ifdef _USE_NCURSES_

GUIFactory::GUIFactory() {
} ;


I_GUIWindowImp &GUIFactory::CreateWindowImp(GUICreateWindowParams &p) {
	return *(new NCGUIWindowImp(p)) ;
}

EventManager *GUIFactory::GetEventManager() {
	return NCEventManager::GetInstance() ;
}
#endif
