/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _T_SINGLETON_H_
#define _T_SINGLETON_H_

// #include "SingletonRegistry.h"
#include "T_SimpleList.h"

class I_Singleton {
public:
  virtual ~I_Singleton(){};
};

template <class Item> class T_Singleton : public I_Singleton {
protected:
  T_Singleton();
  virtual ~T_Singleton();

public:
  // Get the currently installed factory

  static Item *GetInstance();

protected:
  // The static instance of the singleton

  static Item *instance_;
};

#include "T_Singleton.cpp" // Include the implementation file.

#endif
