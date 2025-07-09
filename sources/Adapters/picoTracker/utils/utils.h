/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _PICOTRACKERUTIL_H_
#define _PICOTRACKERUTIL_H_

#include "pico/stdlib.h"

/*
**  Bit set, clear, and test operations
**
**  public domain snippet by Bob Stout
*/

typedef enum { ERROR = -1, FALSE, TRUE } LOGICAL;

#define BOOL(x) (!(!(x)))

#define BitSet(arg, posn) ((arg) | (1L << (posn)))
#define BitClr(arg, posn) ((arg) & ~(1L << (posn)))
#define BitTst(arg, posn) BOOL((arg) & (1L << (posn)))
#define BitFlp(arg, posn) ((arg) ^ (1L << (posn)))

uint32_t measure_free_mem(void);
void measure_freqs(void);
#ifdef SDIO_BENCH
void sd_bench();
#endif
#endif
