/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _AUDIO_MODULE_H_
#define _AUDIO_MODULE_H_

#include "Application/Utils/fixed.h"
#include "Foundation/Types/Types.h"

class AudioModule {
public:
  virtual ~AudioModule(){};
  virtual bool Render(fixed *buffer, int samplecount) = 0;
};

#endif
