/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _MULTI_CHANNEL_ADAPTER_H_
#define _MULTI_CHANNEL_ADAPTER_H_

#include "Foundation/Observable.h"
#include "Foundation/T_SimpleList.h"
#include "Services/Controllers/Channel.h"

class MultiChannelAdapter : T_SimpleList<Channel>,
                            public I_Observer,
                            public Channel {
public:
  MultiChannelAdapter(const char *name, bool owner = false);
  virtual ~MultiChannelAdapter();
  bool AddChannel(const char *decription);
  void AddChannel(Channel &);
  virtual void SetValue(float value, bool notify = true);

private:
  virtual void Update(Observable &o, I_ObservableData *d);
};
#endif
