/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "Observable.h"

Observable::Observable() { _hasChanged = false; }

Observable::Observable(etl::ivector<I_Observer *> *list) : _list(list) {
  _hasChanged = false;
}

Observable::~Observable() {}

void Observable::AddObserver(I_Observer &o) {
  if (_list != NULL) {
    _list->push_back(&o);
  } else {
    _variable = &o;
  }
}

void Observable::RemoveObserver(I_Observer &o) {
  if (_list != NULL) {
    etl::ivector<I_Observer *>::iterator it = _list->begin();
    while (it != _list->end()) {
      if (*it == &o) {
        _list->erase(it);
        break;
      }
      it++;
    }
  } else {
    _variable = NULL;
  }
}

void Observable::RemoveAllObservers() {
  if (_list != NULL) {
    etl::ivector<I_Observer *>::iterator it = _list->begin();
    while (it != _list->end()) {
      it = _list->erase(it);
    }
  } else {
    _variable = NULL;
  }
}

void Observable::NotifyObservers(I_ObservableData *d) {
  if (_hasChanged) {
    if (_list != NULL) {
      etl::ivector<I_Observer *>::iterator it = _list->begin();
      while (it != _list->end()) {
        I_Observer *o = *it++;
        o->Update(*this, d);
      }
    } else if (_variable != NULL) {
      _variable->Update(*this, d);
    }
    ClearChanged();
  }
}

void Observable::SetChanged() { _hasChanged = true; }

bool Observable::HasChanged() { return _hasChanged; }
