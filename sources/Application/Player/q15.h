// adapted from https://github.com/ARM-software/CMSIS_5
/*
Copyright (C) 2010-2022 ARM Limited or its affiliates. All rights reserved.
Christophe Favergeon
*/

#ifndef _FIXED_POINT_Q15_H_
#define _FIXED_POINT_Q15_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __STATIC_FORCEINLINE
#define __STATIC_INLINE static inline
#endif

#include <stdint.h>

/**
 * @brief 8-bit fractional data type in 1.7 format.
 */
typedef int8_t q7_t;

/**
 * @brief 16-bit fractional data type in 1.15 format.
 */
typedef int16_t q15_t;

/**
 * @brief 32-bit fractional data type in 1.31 format.
 */
typedef int32_t q31_t;

__STATIC_INLINE int32_t __SSAT(int32_t val, uint32_t sat) {
  if ((sat >= 1U) && (sat <= 32U)) {
    const int32_t max = (int32_t)((1U << (sat - 1U)) - 1U);
    const int32_t min = -1 - max;
    if (val > max) {
      return max;
    } else if (val < min) {
      return min;
    }
  }
  return val;
}

#define Q31_MAX ((q31_t)(0x7FFFFFFFL))
#define Q15_MAX ((q15_t)(0x7FFF))
#define Q7_MAX ((q7_t)(0x7F))
#define Q31_MIN ((q31_t)(0x80000000L))
#define Q15_MIN ((q15_t)(0x8000))
#define Q7_MIN ((q7_t)(0x80))

#define Q31_ABSMAX ((q31_t)(0x7FFFFFFFL))
#define Q15_ABSMAX ((q15_t)(0x7FFF))
#define Q7_ABSMAX ((q7_t)(0x7F))
#define Q31_ABSMIN ((q31_t)0)
#define Q15_ABSMIN ((q15_t)0)
#define Q7_ABSMIN ((q7_t)0)

inline q15_t add_q15(q15_t srcA, q15_t srcB) {
  return (q15_t)__SSAT(((q31_t)srcA + srcB), 16);
}
inline q15_t mult_q15(q15_t srcA, q15_t srcB) {
  q31_t mul = (q31_t)((q15_t)(srcA) * (q15_t)(srcB));
  return (q15_t)__SSAT(mul >> 15, 16);
}
inline q15_t sub_q15(q15_t srcA, q15_t srcB) {
  return (q15_t)__SSAT(((q31_t)srcA - srcB), 16);
}
inline q15_t f32_to_q15(float in) { return (q15_t)(in * 32767.0f); }
inline float q15_to_f32(q15_t in) { return ((float)in / 32767.0f); }
inline q15_t lerp_q15(q15_t a, q15_t b, q15_t amt) {
  return add_q15(a, mult_q15(sub_q15(a, b), amt));
}

#ifdef __cplusplus
}
#endif

#endif // _FIXED_POINT_Q15_H_