/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "utils.h"
#include "System/System/System.h"
#include <System/Console/Trace.h>
#include <stdio.h>

uint32_t measure_free_mem(void) {
  void *buff[256];
  uint32_t max = 0;

  int i = 0;
  for (; i < 256; i++) {
    buff[i] = malloc(1000);
    if (buff[i]) {
      max = i;
    } else {
      break;
    }
  }
  for (int j = i; j >= 0; j--) {
    free(buff[j]);
  }

  Trace::Debug("MAX memory free in heap: %i\n", max * 1000);
  /*
    buff = malloc(80000);
  if (buff) {
    Trace::Debug("MALLOC addr: %p %i - Mem free: %i\n", buff,
           reinterpret_cast<uintptr_t>(buff),  0x20040000l -
               reinterpret_cast<uintptr_t>(buff));
    free(buff);
    }*/
  return max;
}
