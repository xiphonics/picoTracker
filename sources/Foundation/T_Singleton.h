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

#include "Externals/etl/include/etl/singleton.h"

class I_Singleton {
public:
  virtual ~I_Singleton(){};
};

template <class Item> class T_Singleton : public I_Singleton {
protected:
  T_Singleton();
  virtual ~T_Singleton();
  T_Singleton(const T_Singleton &) = delete;
  T_Singleton &operator=(const T_Singleton &) = delete;

public:
  // Get the currently installed factory

  static Item *GetInstance();
  template <typename... TArgs> static Item *Create(TArgs &&...args);
};

#include "T_Singleton.cpp" // Include the implementation file.

#endif
