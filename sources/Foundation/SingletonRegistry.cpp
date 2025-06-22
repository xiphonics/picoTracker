/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "SingletonRegistry.h"

SingletonRegistry _instance;

SingletonRegistry::SingletonRegistry() : T_SimpleList<I_Singleton>(true){};

SingletonRegistry::~SingletonRegistry(){};

SingletonRegistry *SingletonRegistry::GetInstance() { return &_instance; };
