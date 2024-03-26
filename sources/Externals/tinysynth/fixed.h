// fixed point arithmatic

#ifndef _FIXED_H
#define _FIXED_H

#define USE_FIXED_POINT 1

#ifdef USE_FIXED_POINT

#define FIXED_SHIFT 15
#define FIXED_SCALE (1 << FIXED_SHIFT)

#define i2fp(a) ((a) << FIXED_SHIFT)
#define fp2i(a) ((a) >> FIXED_SHIFT)

#define fp_add(a, b) ((a) + (b))
#define fp_sub(a, b) ((a) - (b))

// #define FP_FRACMASK 0x10007FFF ; // ! Only valid with FIXED_SHIFT=15

#if defined(PLATFORM_GP32) || defined(PLATFORM_GP2X)

#define fp_mul(x, y)                                                           \
  ({                                                                           \
    int __hi;                                                                  \
    unsigned int __lo;                                                         \
    fixed __result;                                                            \
    asm("smull	%0, %1, %3, %4\n\t"                                             \
        "movs	%0, %0, lsr %5\n\t"                                              \
        "adc	%2, %0, %1, lsl %6"                                               \
        : "=&r"(__lo), "=&r"(__hi), "=r"(__result)                             \
        : "%r"(x), "r"(y), "M"(FIXED_SHIFT), "M"(32 - FIXED_SHIFT)             \
        : "cc");                                                               \
    __result;                                                                  \
  })

#else

#define fp_mul(x, y) ((fixed)(((long long)(x) * (long long)(y)) >> FIXED_SHIFT))
#endif

#define fp_div(x, y) ((((x) << 2) / ((y) >> 8)) << 10)
#define fp_mulint(x, y) fp_mul(x, y)

#define FP_ONE (1 << FIXED_SHIFT)
#define FPONE FP_ONE

typedef signed int fixed;

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
