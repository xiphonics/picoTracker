
#ifndef _I_ITERATOR_H_
#define _I_ITERATOR_H_

//
// I_Iterator: Defines the interface all iterators should respect. That
// way, changing the collection implementation should have a minimal impact
// on client code.
//
// Typical client code:
// -------------------------------------------------------------
// I_Iterator *it=container.GetIterator() ;
//
// for (it->Begin();!it->IsDone();it->Next()) {
//      .. it->CurrentItem() ;
//      ..
// }
//
// delete it ;
// -------------------------------------------------------------

template <class Item>
class I_Iterator {
public:

	virtual ~I_Iterator() {} ;
	// Resets the iterator to the first item
	virtual void Begin() = 0 ;
	// Go to the next item... assume there is one more
	virtual void Next() = 0 ;
	// Returns whether the iterator contains any more elements
	virtual bool IsDone() const = 0 ;
	// Returns the current item 
    virtual Item& CurrentItem() const = 0 ; 

protected:
	// This is an interface, we don't want to let it instanciated
	I_Iterator() {} ;
} ;

#endif
