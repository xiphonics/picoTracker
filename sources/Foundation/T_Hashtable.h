
#ifndef _T_HASHTABLE_H_
#define _T_HASHTABLE_H_

#include "I_Hashkey.h"
#include "T_Vector.h"
#include "IteratorPtr.h"

#include <stdio.h>

// A hashtable. The hashtable is keeping objects associated to keys and
// allow fast lookup according to the key. Keys can be of various type, they
// just need to implement the I_Hashkey interface.
//
// Important note: the Hashtable is NOT doing content cleanup i.e. when it
// is destroyed, it does not attempt to free the object it contents. If needed
// the client code needs to get an iterator to the content and implement the
// destruction code

template <class Item> class T_HashtableIterator ;

template <class Item>
class T_Hashtable {

friend class T_HashtableIterator<Item> ; // For iterator efficiency, we make it friend

public:

	// Constructor

	T_Hashtable() ;

	// Destructor

	~T_Hashtable() ;

	// Adds an Item in the hash table. If there is already an item associated to
	// that element, it is NOT replaced ; the original item stays.

	void Put(I_Hashkey &key,Item &item) ;

	// Finds the item associated to the specified key
    // Returns NULL if there is not item for that key

	Item *Get(I_Hashkey &key) ;

	// Removes the item associated to the specified key

    void Remove(I_Hashkey &key) ;

	// Empties the hashtable content

	void Empty() ;

	// Gets an iterator on the content of the hashtable

//	I_Iterator<Item> *GetIterator() ;

	T_HashtableIterator<Item> *GetIterator() ;


protected:
	enum EntryType { ACTIVE, EMPTY, DELETED } ; // State of the hashtable cells
private:
	int findPos(I_Hashkey &key) ;               // Finds the internal cell position associated to the specified key
	bool isActive(int) ;                        // Specifies if the specified cell is currently in use
	void rehash() ;                             // Expands the internal storage allocated to the hashtable
	static bool isPrime(int) ;                  // Basic algorithm to find a prime number (because the storage size should always be a prime number)
	static int nextPrime(int) ;

private:
    struct HashEntry                            // One cell in the hashtable. Allow storage of key,element and a state in which the cell is
    {

	  // Constructor
	  HashEntry(I_Hashkey *key=NULL,Item *item=NULL,EntryType info=EMPTY){
		  if (key!=NULL) {
			_key=&(key->Clone()) ; // to avoid mess, keys are always cloned so there's no sharing and we can destroy it
		  } else {
			_key=NULL ;
		  }
		  _item=item ;
		  _info=info ; 
	  } ;
	  // Destructor
	  ~HashEntry() {
		  if (_key!=NULL) delete _key ;
	  } ;
	  // Copy constructor
	  HashEntry(const HashEntry &other) {
		  _info=other._info ;
		  _item=other._item ;
		  _key=other._key->Clone() ;
	  } ;
	  // Assignment operator (needed for the vector storage)
	  void operator=(const HashEntry &other) {
		  if (this!=&other) {
			  if (_key!=NULL) {
				  delete _key ;
			  }
			  _info=other._info ;
			  _item=other._item ;
			  _key=&other._key->Clone() ;
		  }
	  }
	  // Accessor for info field
	  inline void SetInfo(EntryType info) {
		  _info=info ;
	  }
	  // Accessor for info field
	  inline EntryType GetInfo() {
		  return _info ;
	  }
	  // Accessor for key field
	  inline I_Hashkey *GetKey() {
		  return _key ;
	  }
	  // Accessor for item field
	  inline Item *GetItem() {
		  return _item ;
	  }
	private:
	  Item *_item ;
	  I_Hashkey *_key ;
	  EntryType _info ;
    };

private:
	T_Vector<HashEntry> *_content ;  // The content of the hashtable
	int _size ;                      // The number of entries so far
} ;

#include "T_Hashtable.cpp"

#endif