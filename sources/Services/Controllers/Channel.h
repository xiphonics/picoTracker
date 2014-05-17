
#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include "Foundation/Observable.h"
#include <string>

class Channel: public Observable {
public:
	Channel(const char *name) ;
	Channel() ;
	virtual ~Channel() ;
	void Trigger() ;
	virtual const char *GetName() ;
	virtual void SetName(const char *) ;
	virtual float GetValue() ;
	virtual void SetValue(float value,bool notify=true) ;
protected:
	std::string name_ ;
	float value_ ;
} ;
#endif
