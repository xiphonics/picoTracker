#ifndef _I_LIST_H_
#define _I_LIST_H_

#include "IteratorPtr.h"

//
// I_List: Defines a generic list interface all container should respect. That
// way we can easily optimise the type of list used for a particular operation.
// Note that when you create a new class that behaves as a list, it is better to
// derive your class from this interface and forward the operation to a standard
// implementation class rather than deriving directly from the implementation class
//
// template <class MyType>
// class MyBag:public I_List<MyType> {
//
// public:
//   Insert(MyType &t) {
//      _list.Insert(t) ;
//   }
//
// private:
//   SimpleList<MyType> _list ;
// }
//

template <class Item>
class I_List
{
  public:

	  virtual ~I_List() {} ;

    // Inserts a new item

	virtual void Insert(Item&) = 0 ;

	// Inserts a new item

	virtual void Insert(Item *) = 0 ;

	// Removes an item

	virtual void Remove(Item&) = 0 ;

	// Returns an iterator on the list content
	// The iterator has to be deleted 

	virtual I_Iterator<Item> *GetIterator() =0 ;

	// Returns true if the list contains the specified Item

	virtual bool Contains(Item&) =0 ;

	// Empty the list content

	virtual void Empty() = 0 ;

  protected:

	// This is an interface so we make sure client code cannot
	// instanciate it.

    I_List() {} ;
};

#endif
