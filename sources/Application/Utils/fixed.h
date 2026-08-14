/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

// fixed point arithmatic

#ifndef _FIXED_H
#define _FIXED_H

#define USE_FIXED_POINT

#ifdef USE_FIXED_POINT

#define FIXED_SHIFT 15
#define FIXED_SCALE (1 << FIXED_SHIFT)

#define i2fp(a) ((a) << FIXED_SHIFT)
#define fp2i(a) ((a) >> FIXED_SHIFT)

typedef signed int fixed;

#define fp_add(a, b) ((a) + (b))
#define fp_sub(a, b) ((a) - (b))

#define fp_mul(x, y) ((fixed)(((long long)(x) * (long long)(y)) >> FIXED_SHIFT))

// fp_mul_coef(x, COEF): equivalent to fp_mul(x, coef) but restricted to cases
// where |COEF| <= FP_ONE. Splitting x into 16-bit halves lets the whole
// computation use 32-bit multiplications instead.
inline fixed fp_mul_coef(fixed x, fixed coef) {
#if FP_MUL_COEF_BASELINE
  return fp_mul(x, coef);
#else
  fixed x_hi = x >> 16;
  fixed x_lo = x & 0xFFFF;
  return (x_hi * coef << 1) + ((x_lo * coef) >> FIXED_SHIFT);
#endif
}

#define fp_div(x, y) ((((x) << 2) / ((y) >> 8)) << 10)
#define fp_mulint(x, y) fp_mul(x, y)

#define FP_ONE (1 << FIXED_SHIFT)
#define FPONE FP_ONE

inline fixed fl2fp(float val) { return (fixed)(val * FIXED_SCALE); }

inline float fp2fl(fixed val) { return ((float)val) / FIXED_SCALE; }

#else // uses float

typedef float fixed;
#define fp2i(a) (int)a
#define i2fp(a) (fixed) a
#define fl2fp(a) a
#define fp2fl(a) a
#define fp_add(a, b) a + b
#define fp_sub(a, b) a - b
#define fp_mul(a, b) a *b
#define fp_div(a, b) a / b

#define FP_ONE 1.0
#define FPONE FP_ONE

#endif

#endif /*_FIXED_H*/
