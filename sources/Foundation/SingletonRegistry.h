/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _SINGLETON_REGISTRY_H_
#define _SINGLETON_REGISTRY_H_

#include "T_SimpleList.h"

class I_Singleton {
public:
  virtual ~I_Singleton(){};
};

class SingletonRegistry : public T_SimpleList<I_Singleton> {
public:
  static SingletonRegistry *GetInstance();

  SingletonRegistry();
  ~SingletonRegistry();
};

#endif
