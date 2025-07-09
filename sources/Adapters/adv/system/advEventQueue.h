/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */
#ifndef _ADVEVENTQUEUE_H_
#define _ADVEVENTQUEUE_H_

#include "Externals/etl/include/etl/deque.h"
#include "Foundation/T_Singleton.h"

enum advEventType { PICO_REDRAW, PICO_FLUSH, PICO_CLOCK, LAST };

class advEvent {
public:
  advEvent(advEventType type) : type_(type) {}
  advEventType type_;
};

inline bool operator==(const advEvent &lhs, const advEvent &rhs) {
  return lhs.type_ == rhs.type_;
};

class advEventQueue : public T_Singleton<advEventQueue> {
public:
  advEventQueue();
  void push(advEvent event);
  void pop_into(advEvent &event);
  bool empty();

private:
  etl::deque<advEvent, advEventType::LAST> queue_;
};

#endif
