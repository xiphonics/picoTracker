#include "GUIFactory.h"
#include "CAANOOGUIWindowImp.h"


GUIFactory::GUIFactory() {
} ;


I_GUIWindowImp &GUIFactory::CreateWindowImp(GUICreateWindowParams &p) {
	return *(new CAANOOGUIWindowImp(p)) ;
}
