/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef TLIST_H
#define TLIST_H

#include "I_List.h"
#include <stdio.h> // for NULL

template <class Item>
struct Node // A node of the list. Provides space to keep the data and keep a
            // pointer to previous and next node
{
  Node(Item &d) : data(d) {
    next = NULL;
    prev = NULL;
  };
  Item &data;
  Node *prev;
  Node *next;
};

//
// T_SimpleList: Implementation of a simple list. Can be used when no specific
// optimal performance is needed. Actual implementation is to use a double
// linked list but that might change

template <class Item> class T_SimpleListIterator;

template <class Item> class T_SimpleList : public I_List<Item> {
public:
  // Constructor: allow to specify if we want the list to own its element or not
  // If it does, it calls its elements destructors when it is destroyed
  T_SimpleList(bool isOwner = false);
  virtual ~T_SimpleList();

  // I_List implementation
  virtual void Insert(Item &);
  virtual void Insert(Item *);
  virtual void Remove(Item &);
  virtual bool Contains(Item &);
  virtual void Empty();
  virtual void Empty(bool reverse);

  // Additional
  Item *GetLast();
  Item *GetFirst();
  void Remove(Item &, bool shouldDelete);
  void SetContent(T_SimpleList<Item> &);

  bool GetOwnership();
  void SetOwnership(bool);

  int Size() { return _size; };

  void Sort();

  // iterator
  virtual void Begin();
  virtual void Next();
  virtual bool IsDone() const;
  virtual Item &CurrentItem() const;

  // Implementation function - keep away !
protected:
  Node<Item> *
  findNode(Item &); // Looks for the node that contains a given item
                    // Note: comparison is on the pointer level at the moment

  void deleteNode(
      Node<Item> *,
      bool owner); // Deletes the specified node and (possibly) its content
  void rewire(Node<Item> *n);
  void exchange(Node<Item> *n1, Node<Item> *n2);

  // Member variable
private:
  Node<Item> *_first; // Pointer to the first node of the list (or NULL if list
                      // is empty)
  Node<Item>
      *_last; // Pointer to the last node of the list (or NULL if list is empty)
  bool _isOwner; // Keep track of the list's ownership on its elements
  int _size;
  // iterator
  Node<Item> *_current;
};

#include "T_SimpleList.cpp" // Include the implementation file.
#endif
