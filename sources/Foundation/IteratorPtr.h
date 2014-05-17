
#ifndef _ITERATORPTR_H_
#define _ITERATORPTR_H_

#include "I_Iterator.h"

//
// IteratorPtr: Helper class to make sure iterators returned by collections
// are correctly destroyed when going out of scope.
//
// Instead of:
//
// I_Iterator<MyType> *it=list.GetIterator() ;
// for (it->Begin();!it->IsDone();it->Next()) {
//
// }
// delete it ;
//
// You can write:
//
// IteratorPtr<MyType> it(list.GetIterator()) ;
// for (it->Begin();!it->IsDone();it->Next()) {
//
// }

template <class Item>
class IteratorPtr {
public:

	// Constructor: we wrap a regular iterator

	IteratorPtr(I_Iterator<Item>* i): _i(i) {} ;

	// Destructor: we make sure the iterator is deleted

	~IteratorPtr() { delete _i ; } ;

	// We declare the pointer operation so that client code
	// is readable as a regular iterator

	I_Iterator<Item>* operator->() { return _i ; } ;
	I_Iterator<Item>& operator*() { return *_i ; }  ;

	// Since we don't want to share the iterators, we prevent
	// copy and assignment

private:
	IteratorPtr(const IteratorPtr&) ;
	IteratorPtr& operator=(const IteratorPtr &) ;
private:
	I_Iterator<Item>* _i ;
} ;

#endif
