
#include "Channel.h"
#include "System/Console/Trace.h"
#include <string.h>
#include <assert.h>
Channel::Channel() {
	name_="unnamed" ;
} ;

Channel::Channel(const char *name) {
	name_=name ;
	value_=0.0F ;
} ;

Channel::~Channel() {
} ;
      
float Channel::GetValue() {
	return value_ ;
} ;

void Channel::SetValue(float value,bool notify) {
	if (!((value>=0.0)&&(value<=1.0))) {
		// assert(0) ;
	} ;
	if (value_!=value) {
		value_=value ;
		if (notify) {
			SetChanged();
		} ;
	}
} ;

void Channel::Trigger() {
	NotifyObservers() ;
}

void Channel::SetName(const char *name) {
	name_=name ;
} ;

const char *Channel::GetName() {
	return name_.c_str() ;
} ;