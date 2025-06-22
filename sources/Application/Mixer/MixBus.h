/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _MIX_BUS_H_
#define _MIX_BUS_H_

#include "Services/Audio/AudioMixer.h"

class MixBus : public AudioMixer {
public:
  MixBus() : AudioMixer("bus"){};
  virtual ~MixBus(){};
};
#endif
