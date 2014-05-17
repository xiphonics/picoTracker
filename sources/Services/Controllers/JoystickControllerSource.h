#ifndef _JOYSTICK_CONTROLLER_SERVICE_H_
#define _JOYSTICK_CONTROLLER_SERVICE_H_

#include "ControllerSource.h"

#define MAX_JOY_CHANNEL_AXIS 10

class JoystickControllerSource: public ControllerSource {
public:
	JoystickControllerSource(const char *name) ;
	virtual ~JoystickControllerSource() ;
	virtual Channel *GetChannel(const char *url) ;
	virtual bool IsRunning() { return true ; } ;
	void SetAxis(int axis,float value) ;
private:
	Channel channel_[MAX_JOY_CHANNEL_AXIS*2] ;
} ;


#endif

