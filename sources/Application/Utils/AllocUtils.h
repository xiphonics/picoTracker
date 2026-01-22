/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _ALLOC_UTILS_H_
#define _ALLOC_UTILS_H_

template <typename IsFreeFn, typename MarkUsedFn>
inline int FindNextAfter(int current, int count, IsFreeFn isFree,
                         MarkUsedFn markUsed) {
  if (count <= 0) {
    return -1;
  }
  int start = (current >= 0 && current < count) ? (current + 1) : 0;
  for (int i = start; i < count; i++) {
    if (isFree(i)) {
      markUsed(i);
      return i;
    }
  }
  for (int i = 0; i < start; i++) {
    if (isFree(i)) {
      markUsed(i);
      return i;
    }
  }
  return -1;
}

#endif
