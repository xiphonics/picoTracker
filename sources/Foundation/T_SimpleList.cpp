
//
// T_SimpleList implementation
//

// Constructor

template <class Item>
T_SimpleList<Item>::T_SimpleList(bool isOwner):I_List<Item>() {
	_isOwner=isOwner ;
	_first = NULL ;
	_last=NULL ;
	_size=0 ;

} 

// Destructor

template <class Item>
T_SimpleList<Item>::~T_SimpleList() {
	Empty() ;
}

// Inserts a new element at the end of the list

template <class Item>
void T_SimpleList<Item>::Insert(Item& item) {
	NAssert(!Contains(item)) ;
	Node<Item> *n=new Node<Item>(item); 
	n->prev=NULL ;
	n->next=NULL ;
	if (_first==NULL) { // We're empty, we add the node
		_first=n ;
		_last=n ;
	} else {            // Else we add it to the tail
		_last->next=n ;
		n->prev=_last ;
		_last=n ;	
	}
	_size++ ;
}

// Inserts a new element at the end of the list (assumes the pointer is not NULL)

template <class Item>
void T_SimpleList<Item>::Insert(Item* item) {
	NAssert(item!=NULL) ;
	this->Insert(*item) ;
}

// Returns true if the list contains the specified item

template <class Item>
bool T_SimpleList<Item>::Contains(Item &item) {
	Node<Item> *node=findNode(item) ;
	return (node==NULL)?false:true ;
}

// Returns a standard iterator for the list

template <class Item>
I_Iterator<Item> *T_SimpleList<Item>::GetIterator() {
	return new T_SimpleListIterator<Item>(*this) ;
}

// Returns a standard iterator for the list

template <class Item>
I_Iterator<Item> *T_SimpleList<Item>::GetIterator(bool reverse) {
	return new T_SimpleListIterator<Item>(*this,reverse) ;
}

// Empties the list's content

template <class Item>
void T_SimpleList<Item>::Empty() {
	Empty(false) ;
}

template <class Item>
void T_SimpleList<Item>::Empty(bool reverse) {

	if (!reverse) {
		Node<Item> *current=_first ;
		while (current!=NULL) {
			Node<Item> *n=current ;
			current=current->next ;
			deleteNode(n,_isOwner) ;
		}
	} else {
		Node<Item> *current=_last ;
		while (current!=NULL) {
			Node<Item> *n=current ;
			current=current->prev ;
			deleteNode(n,_isOwner) ;
		}
	}
	_size=0 ;
	_first=NULL ;
	_last=NULL ;
}

// Removes the element associated to the specified item

template <class Item>
void T_SimpleList<Item>::Remove(Item &i) {
	Node<Item> *n=findNode(i) ;
	if (n!=NULL) {
		deleteNode(n,_isOwner) ;
	}
}

template <class Item>
void T_SimpleList<Item>::Remove(Item &i,bool delContent) {
	Node<Item> *n=findNode(i) ;
	if (n!=NULL) {
		deleteNode(n,delContent) ;
	}
}

// Finds if a node contains the specified item

template <typename Item>
Node<Item> *T_SimpleList<Item>::findNode(Item &item) {
	Node<Item> *current=_first ;
	while (current!=NULL) {
		if (&current->data==&item) {
			return current ;
		}
		current=current->next ;
	} 	
	return NULL ;
}

// Deletes a node and its content if the list has ownership

template <class Item>
void T_SimpleList<Item>::deleteNode(Node<Item> *n,bool isOwner) {

	// Rewire list

	if (n->prev!=NULL) {
		n->prev->next=n->next ;
	} else {
		_first=n->next ;
	}
	if (n->next!=NULL) {
		n->next->prev=n->prev ;
	} else {
		_last=n->prev ;
	}

	// Destroy node data and content if owner

	if (isOwner) {
		Item &i=n->data ;
		delete &i ;
	}
	_size--;
	delete n ;
}

template <class Item>
Item *T_SimpleList<Item>::GetLast() {
	if (_last!=NULL) {
		return &_last->data ;
	} else {
		return NULL;
	}
}

template <class Item>
Item *T_SimpleList<Item>::GetFirst() {
	if (_last!=NULL) {
		return &_first->data ;
	} else {
		return NULL;
	}
}

template <class Item>
void T_SimpleList<Item>::SetContent(T_SimpleList<Item>&content) {
	Empty() ;
	IteratorPtr<Item> it(content.GetIterator()) ;
	for (it->Begin();!it->IsDone();it->Next()) {
		Insert(it->CurrentItem()) ;
	}
}

/*
template <class Item>
void T_SimpleList<Item>::GetContent(T_SimpleList<Item>&content) {
	content.Empty() ;
	IteratorPtr<USQEventControl> it(GetIterator()) ;
	for (it->Begin();!it->IsDone();it->Next()) {
		content.Insert(it->CurrentItem()) ;
	}
} */

template <class Item>
bool T_SimpleList<Item>::GetOwnership() {
	return _isOwner ;
}

template <class Item>
void T_SimpleList<Item>::SetOwnership(bool isOwner) {
	_isOwner=isOwner ;
}

template <class Item>
void T_SimpleList<Item>::Sort() {
	if (_first==_last) return ;
	Node<Item>* current=_first ;
	Node<Item>* next=current->next ;
	while (next!=0) {
		int result=current->data.Compare(next->data) ;
		if (result>0) {
			exchange(current,next) ;
			Node<Item> *temp=next ;
			Node<Item> *prev=next->prev ;
			while (prev!=0) {
				if (temp->data.Compare(prev->data)>=0) {
					break ;
				} ;
//			    Trace::Debug("Swapping %s and %s",temp->data.GetPath(),prev->data.GetPath()) ;
				exchange(temp,prev) ;
				prev=temp->prev ;
			} ;
		} else {
			current=next ;
		};
		next=current->next ;
	} ;
}

template <class Item>
void T_SimpleList<Item>::rewire(Node<Item>*n) {
	if (n->prev) n->prev->next=n ;
	if (n->next) n->next->prev=n ;
} ;

template <class Item>
void T_SimpleList<Item>::exchange(Node<Item>*n1,Node<Item>*n2) {

	Node<Item>*temp ;

	if (n1->prev==n2) {
		n2->next=n1->next ;
		n1->prev=n2->prev;
		n1->next=n2 ;
		n2->prev=n1 ;
	} else if (n2->prev==n1) {
		n1->next=n2->next ;
		n2->prev=n1->prev;
		n2->next=n1 ;
		n1->prev=n2 ;
	} else {
		temp=n1->prev ;
		n1->prev=n2->prev ;
		n2->prev=temp ;

		temp=n1->next ;
		n1->next=n2->next ;
		n2->next=temp ;
	}

	rewire(n1) ;
	rewire(n2) ;

	// Check for first and last
	if (n1==_first) {
		_first=n2 ;
	} else {
		if (n2==_first) {
			_first=n1 ;
		} ;
	}
	if (n1==_last) {
		_last=n2 ;
	} else {
		if (n2==_last) {
			_last=n1 ;
		} ;
	}
} ;
