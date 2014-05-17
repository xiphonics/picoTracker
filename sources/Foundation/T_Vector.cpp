
//
// Implementation of the T_Vector class
//

// Constructor

template <class Item>
T_Vector<Item>::T_Vector(int initialSize) {
	_content=new Item[initialSize];  // Initializes the array of elements we hold
	_size=initialSize ;
}

// Destructor

template <class Item>
T_Vector<Item>::~T_Vector() {
	delete[] _content ;
}

// Inserts a new element at the specified position

template <class Item>
void T_Vector<Item>::Put(int index,Item& item) {
	_content[index]=item ; ;
}

// Re adapt the size available to the vector

template <class Item>
void T_Vector<Item>::Resize(int size) {
	Item* oldContent=_content ;
	_content=new Item[size] ;
	int tSize=(size<_size)?size:_size ;
	for (int i=0;i<tSize;i++) {
		_content[i]=oldContent[i] ;
	};
	_size=size ;
	delete[] oldContent ;

}
