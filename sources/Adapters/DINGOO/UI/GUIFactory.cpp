#include "GUIFactory.h"
#include "DINGOOGUIWindowImp.h"


GUIFactory::GUIFactory() {
} ;


I_GUIWindowImp &GUIFactory::CreateWindowImp(GUICreateWindowParams &p) {
	return *(new DINGOOGUIWindowImp(p)) ;
}
