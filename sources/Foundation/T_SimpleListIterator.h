#ifndef _SIMPLELISTITERATOR_H_
#define _SIMPLELISTITERATOR_H_

#include "I_Iterator.h"
#include "T_SimpleList.h"

//
// T_SimpleListIterator: An iterator that traverses the content of a T_SimpleList
// and complies with the generic I_Iterator interface
//

template <class Item>
class T_SimpleListIterator: public I_Iterator<Item> {
public:

	// Constructor - reverse enumerates item from tail to head

	T_SimpleListIterator(T_SimpleList<Item>& list,bool reverse=false) ;

	virtual ~T_SimpleListIterator(){} ;

	// Resets the iterator at the beginning of the list

	virtual void Begin() ;

	// Goes to the next element in the list.. assumes there is one

	virtual void Next() ;

	// Tells if the iterator has exhausted the list content

	virtual bool IsDone() const ;

	// Return the current element of the list

	virtual Item& CurrentItem() const ;

private:
	T_SimpleList<Item>& _list ;              // The list we work on
	Node<Item> *_current ;     // The Node containing the current element
	bool _reverse ;

} ;

#include "T_SimpleListIterator.cpp"
#endif
