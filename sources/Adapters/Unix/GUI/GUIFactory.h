#ifndef NCURSES_GUI_FACTORY_H_
#define NCURSES_GUI_FACTORY_H_

#ifdef _USE_NCURSES_

#include "UIFramework/Interfaces/I_GUIWindowFactory.h"

class GUIFactory: public I_GUIWindowFactory {

public:

	GUIFactory() ;
	virtual I_GUIWindowImp &CreateWindowImp(GUICreateWindowParams &) ;
	virtual EventManager *GetEventManager() ;

} ;

#endif

#endif
