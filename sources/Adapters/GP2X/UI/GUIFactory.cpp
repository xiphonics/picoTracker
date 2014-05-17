#include "GUIFactory.h"
#include "GP2XGUIWindowImp.h"


GUIFactory::GUIFactory() {
} ;


I_GUIWindowImp &GUIFactory::CreateWindowImp(GUICreateWindowParams &p) {
	return *(new GP2XGUIWindowImp(p)) ;
}
