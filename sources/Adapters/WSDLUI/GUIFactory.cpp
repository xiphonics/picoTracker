#include "GUIFactory.h"
#include "WSDLGUIWindowImp.h"


GUIFactory::GUIFactory() {
} ;


I_GUIWindowImp &GUIFactory::CreateWindowImp(GUICreateWindowParams &p) {
	return *(new WSDLGUIWindowImp(p)) ;
}
