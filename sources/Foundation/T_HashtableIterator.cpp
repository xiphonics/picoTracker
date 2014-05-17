#include <stdio.h>

//
// Implementation for the hashtable iterator
//

// constructor

template <class Item>
T_HashtableIterator<Item>::T_HashtableIterator(T_Hashtable<Item> &t):_table(t) {
}

// initializes the iterator to the begin of the set

template <class Item>
void T_HashtableIterator<Item>::Begin() {
	_currentIndex=0 ;
	findNextEntry() ;
}

// go to the next element

template <class Item>
void T_HashtableIterator<Item>::Next() {
	findNextEntry() ;
}

// Returns if the whole set has been traversed

template <class Item>
bool T_HashtableIterator<Item>::IsDone() const {
	return (_currentEntry==NULL) ;
}

// Returns the item of the current element

template <class Item>
Item& T_HashtableIterator<Item>::CurrentItem() const {
	return *(_currentEntry->GetItem()) ;
}

template <class Item>
I_Hashkey& T_HashtableIterator<Item>::CurrentKey() const {
	return *(_currentEntry->GetKey()) ;
}

// Look for the next active element starting from the position
// of _currentIndex. Scans the table until an ACTIVE elment has been found
// or the table content has been exhausted

template <class Item>
void T_HashtableIterator<Item>::findNextEntry() {
	int tableSize=_table._content->Size() ;
	while ((_currentIndex<tableSize)&&(_table._content->Get(_currentIndex).GetInfo()!=T_Hashtable<Item>::ACTIVE)) {
		_currentIndex++ ;
	}
	if (_currentIndex<tableSize) {
		_currentEntry=&_table._content->Get(_currentIndex) ;
	} else {
		_currentEntry=NULL ;
	}
	_currentIndex++ ;
}