#include "JoystickControllerSource.h"
#include "System/Console/Trace.h"

JoystickControllerSource::JoystickControllerSource(const char *name):ControllerSource("joy",name) {
} ;

JoystickControllerSource::~JoystickControllerSource() {
} ;

Channel *JoystickControllerSource::GetChannel(const char *url) {
	int axis=url[0]-'0' ;
	int side=(url[1]=='+')?1:0 ;
	return channel_+axis*2+side ;
}

// value = -1 -> 1

void JoystickControllerSource::SetAxis(int axis,float value) {
	int base=axis*2 ;

	channel_[base].SetValue((value<-0.5)?1.0f:0.0f) ;
	channel_[base+1].SetValue((value>0.5)?1.0f:0.0f) ;
	channel_[base].NotifyObservers() ;
	channel_[base+1].NotifyObservers() ;
}
