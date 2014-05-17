
#include "MultiChannelAdapter.h"
#include "Services/Controllers/ControllerService.h"

MultiChannelAdapter::MultiChannelAdapter(const char *name,bool owner):Channel(name),T_SimpleList<Channel>(owner) {
} ;

MultiChannelAdapter::~MultiChannelAdapter() {
	IteratorPtr<Channel> it(GetIterator()) ;
	for (it->Begin();!it->IsDone();it->Next()) {
		Channel &current=it->CurrentItem() ;
		current.RemoveObserver(*this)  ;
	} ;
} ;

bool MultiChannelAdapter::AddChannel(const char *path) {
	Channel *channel=ControllerService::GetInstance()->GetChannel(path) ;
	if (channel) {
		Insert(channel) ;
		channel->AddObserver(*this) ;
	} ;
	return (channel!=0) ;
} ;

void MultiChannelAdapter::AddChannel(Channel &channel) {
	Insert(channel) ;
	channel.AddObserver(*this) ;
} ;

void MultiChannelAdapter::Update(Observable &o,I_ObservableData *d) {
	Channel &source=(Channel &)o ;

	value_=source.GetValue() ;
	IteratorPtr<Channel> it(GetIterator()) ;
	for (it->Begin();!it->IsDone();it->Next()) {
		Channel &current=it->CurrentItem() ;
		if (&current!=&source) {
			current.SetValue(value_,false) ;
		} ;
	} ;
	SetChanged() ;
	NotifyObservers() ;
} ;

void MultiChannelAdapter::SetValue(float value,bool notify) {

	value_=value ;
	IteratorPtr<Channel> it(GetIterator()) ;
	for (it->Begin();!it->IsDone();it->Next()) {
		Channel &current=it->CurrentItem() ;
		current.SetValue(value,false) ;
	} ;
	if (notify) {
		SetChanged() ;
		NotifyObservers() ;
	} ;
}

