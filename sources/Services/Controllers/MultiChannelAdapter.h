
#ifndef _MULTI_CHANNEL_ADAPTER_H_
#define _MULTI_CHANNEL_ADAPTER_H_

#include "Services/Controllers/Channel.h"
#include "Foundation/T_SimpleList.h"
#include "Foundation/Observable.h"

class MultiChannelAdapter: T_SimpleList<Channel>,public I_Observer, public Channel {
public:
	MultiChannelAdapter(const char *name, bool owner=false) ;
	virtual ~MultiChannelAdapter() ;
	bool AddChannel(const char *decription) ;
	void AddChannel(Channel &) ;
	virtual void SetValue(float value,bool notify=true) ;

private:
    virtual void Update(Observable &o,I_ObservableData *d) ;
} ;
#endif

