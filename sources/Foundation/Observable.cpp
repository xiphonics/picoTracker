
#include "Observable.h"

#include "T_SimpleList.h"

Observable::Observable() {
	_hasChanged=false ;
}

Observable::~Observable() {
}

void Observable::AddObserver(I_Observer &o) {
	_list.push_back(&o) ;
}

void Observable::RemoveObserver(I_Observer &o) {
	std::vector<I_Observer *>::iterator it=_list.begin() ;
	while (it!=_list.end()) {
		if (*it==&o) {
			_list.erase(it) ;
			break ;
		}
		it++ ;
	}
}

void Observable::RemoveAllObservers() {
	std::vector<I_Observer *>::iterator it=_list.begin() ;
	while (it!=_list.end()) {
		it=_list.erase(it) ;
	}
}

void Observable::NotifyObservers(I_ObservableData *d) {
	if (_hasChanged) {
		std::vector<I_Observer *>::iterator it=_list.begin() ;
		while (it!=_list.end()) {
			I_Observer *o=*it++ ;
			o->Update(*this,d) ;
		}
		ClearChanged() ;
	}







}

void Observable::SetChanged() {
	_hasChanged=true ;
}

bool Observable::HasChanged() {
	return _hasChanged ;
}
