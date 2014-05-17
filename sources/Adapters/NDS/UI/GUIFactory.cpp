#include "GUIFactory.h"
#include "NDSGUIWindowImp.h"


GUIFactory::GUIFactory() {
} ;


I_GUIWindowImp &GUIFactory::CreateWindowImp(GUICreateWindowParams &p) {
	return *(new NDSGUIWindowImp(p)) ;
}
