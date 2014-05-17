#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "UIFramework/SimpleBaseClasses/GUIWindow.h"
#include "Foundation/T_Singleton.h"

class Application:public T_Singleton<Application> {

public:
	Application() ;
	~Application() ;
	bool Init(GUICreateWindowParams &params) ;

	GUIWindow *GetWindow() ;
protected:
  void initMidiInput();

private:
	GUIWindow *window_ ;
private:
	static Application* instance_ ;
} ;

#endif

