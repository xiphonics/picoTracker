
#ifndef _T_HASHTABLEITERATOR_H_
#define _T_HASHTABLEITERATOR_H_

#include "I_Iterator.h"
#include "T_Hashtable.h"

// An iterator for the hashtable content. Implements the generic iterator interface

template <class Item>
class T_HashtableIterator: public I_Iterator<Item> {
public:

	// Constructor. Takes the hashtable to be transversed

	T_HashtableIterator(T_Hashtable<Item>&) ;

	// Reset the iterator to the beginning of the set

	virtual void Begin()  ;

    // Go to the next element in the set

	virtual void Next() ;

    // Returns true if the end of the set has been reached.

	virtual bool IsDone() const  ;

	// Returns the current element of the iteration. Assumes the end of
	// the set has not been reached.

    virtual Item& CurrentItem() const ; 

	// Returns the key associated to the current element in the iteration

	I_Hashkey& CurrentKey() const ;

private:
	void findNextEntry() ;  // Scans the hashtable looking for the next active element
private:
	T_Hashtable<Item>& _table ; // The table we work with
    T_Hashtable<Item>::HashEntry *_currentEntry ; // The current active entry
	int _currentIndex ;

} ;

#include "T_HashtableIterator.cpp"

#endif