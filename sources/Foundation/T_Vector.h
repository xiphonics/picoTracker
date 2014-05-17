
#ifndef _T_VECTOR_H_
#define _T_VECTOR_H_

#include "I_Iterator.h"

//
// T_Vector: a *VERY* simple template vector implementation. At the current stage; the
// following limitations apply:
//
// 1. The datatype stored in the vector has to have a blank constructor
// 2. The vector uses the assignement operator (=). Be sure your data supports it
// 3. There is no check to see if the index operations are inside the size of the
//    Vector. Access outside bounds might cause exception
//
// The vector holds copy of the data rather than the data passed

template <class Item>
class T_Vector {
public:

	// The constructor: specify the initial size

	T_Vector(int initialSize) ;

	// Destructor

	~T_Vector() ;

	// Returns the vector size

	inline int Size() {  return _size ; } ;

	// Returns the item at the specified index. If no element has been set
	// at that position, a blank item of the specified datatype is returned

	inline Item &Get(int index) { return _content[index] ; } ;

	// Puts an element at the specified position

	void Put(int index,Item &item) ;

	// Modify the storage size available to the vector

	void Resize(int size) ;
private:
	Item *_content ;        // The space for the storage
	int _size ;             // The size of the content
} ;

#include "T_Vector.cpp"

#endif
