#ifndef _BUTTON_CONTROLLER_SOURCE_H_
#define _BUTTON_CONTROLLER_SOURCE_H_

#include "ControllerSource.h"

#define MAX_BUTTON 25

class ButtonControllerSource: public ControllerSource {
public:
	ButtonControllerSource(const char *name) ;
	virtual ~ButtonControllerSource() ;
	virtual Channel *GetChannel(const char *url) ;
	virtual bool IsRunning() { return true ; } ;
	void SetButton(int button,bool value) ;
private:
	Channel channel_[MAX_BUTTON] ;
} ;

#endif

