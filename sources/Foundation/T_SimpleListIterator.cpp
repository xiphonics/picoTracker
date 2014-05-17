#include "System/Console/n_assert.h"

//
// Implementation of the Iterator working on the T_SimpleList
//

template <class Item>
T_SimpleListIterator<Item>::T_SimpleListIterator(T_SimpleList<Item>& list,bool reverse):_list(list),_reverse(reverse) {
	Begin() ;
}

template <class Item>
void T_SimpleListIterator<Item>::Begin() {
	if (_reverse) {
		_current=_list._last ;
	} else {
		_current=_list._first ;
	}

}

template <class Item>
void T_SimpleListIterator<Item>::Next() {
	NAssert(!IsDone()) ;
	if (_reverse) {
		_current=_current->prev ;
	} else {
		_current=_current->next ;
	}
}

template <class Item>
bool T_SimpleListIterator<Item>::IsDone() const {
	return (_current==NULL) ;
}

template <class Item>
Item &T_SimpleListIterator<Item>::CurrentItem() const {
	NAssert(!IsDone()) ;
	return _current->data ;
}
