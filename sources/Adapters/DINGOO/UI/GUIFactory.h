#ifndef DINGOO_GUI_FACTORY_H_
#define DINGOO_GUI_FACTORY_H_

#include "UIFramework/Interfaces/I_GUIWindowFactory.h"

class GUIFactory: public I_GUIWindowFactory {



public:

	GUIFactory() ;

	virtual I_GUIWindowImp &CreateWindowImp(GUICreateWindowParams &) ;

} ;


#endif
