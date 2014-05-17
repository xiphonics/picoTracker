
#ifndef _I_GUIWINDOWFACTORY_H_
#define _I_GUIWINDOWFACTORY_H_

#include "I_GUIWindowImp.h"
#include "UIFramework/SimpleBaseClasses/EventManager.h"

#include "UIFramework/BasicDatas/GUICreateWindowParams.h"
#include "Foundation/T_Factory.h"

// The GUI factory. This object is responsible for the creation of all
// system implementations like WindowImps, BitmapImps etc. When the user
// interface is running, only one factory is available. For each supported
// system, a subclass must be created, returning imp objects derived from the
// specified interfaces. The factory to be used can be installed at run time.

class I_GUIWindowFactory: public T_Factory<I_GUIWindowFactory> {

protected:

	I_GUIWindowFactory() {} ;
	virtual ~I_GUIWindowFactory() {} ;

public:

	// Creates a window system implementation from the specified creational
	// parameters

	virtual I_GUIWindowImp &CreateWindowImp(GUICreateWindowParams &)=0 ;
	virtual EventManager *GetEventManager()=0 ;

} ;
#endif
