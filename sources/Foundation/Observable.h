/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#pragma once

#include "Externals/etl/include/etl/vector.h"

// Data to be passed from the observable to the observer

class I_ObservableData {};

// The observer: Simply allows to be notified with data

class Observable;

class I_Observer {
public:
  virtual ~I_Observer(){};
  virtual void Update(Observable &o, I_ObservableData *d) = 0;
};

// The observable

class Observable {
public:
  Observable();
  Observable(etl::ivector<I_Observer *> *list);
  virtual ~Observable();
  void AddObserver(I_Observer &o);
  void RemoveObserver(I_Observer &o);
  void RemoveAllObservers();
  int CountObservers();

  inline void NotifyObservers() { NotifyObservers(0); };

  void NotifyObservers(I_ObservableData *d);

  void SetChanged();
  inline void ClearChanged() { _hasChanged = false; };
  bool HasChanged();

private:
  etl::ivector<I_Observer *> *_list = NULL;
  I_Observer *_variable = NULL;
  bool _hasChanged;
};
