#include "GUIFactory.h"
#include "DEBGUIWindowImp.h"


GUIFactory::GUIFactory() {
} ;


I_GUIWindowImp &GUIFactory::CreateWindowImp(GUICreateWindowParams &p) {
	return *(new DEBGUIWindowImp(p)) ;
}
