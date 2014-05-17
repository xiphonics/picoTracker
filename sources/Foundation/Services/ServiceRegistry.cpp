
#include "ServiceRegistry.h"

void ServiceRegistry::Register(Service *s) {
    services_.Insert(s) ;
} ;

void ServiceRegistry::Register(SubService *s) {
	IteratorPtr<Service> it(services_.GetIterator()) ;
	for (it->Begin();!it->IsDone();it->Next()) {
		Service &current=it->CurrentItem() ;
		if (current.GetFourCC()==s->GetFourCC()) {
			current.Register(s) ;
		} ;
	} ;
} ;

void ServiceRegistry::Unregister(SubService *s) {
	IteratorPtr<Service> it(services_.GetIterator()) ;
	for (it->Begin();!it->IsDone();it->Next()) {
		Service &current=it->CurrentItem() ;
		if (current.GetFourCC()==s->GetFourCC()) {
			current.Unregister(s) ;
		} ;
	} ;
} ;

