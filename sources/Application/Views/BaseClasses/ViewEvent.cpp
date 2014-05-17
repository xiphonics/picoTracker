
#include "ViewEvent.h"

ViewEvent::ViewEvent(ViewEventType type,void *data) {
	type_=type ;
	data_=data ;
} ;

ViewEventType ViewEvent::GetType() {
	return type_ ;
} ;

void *ViewEvent::GetData() {
	return data_ ;
} ;
