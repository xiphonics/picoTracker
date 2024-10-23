#ifndef MACROS_H_
#define MACROS_H_

#define CLIP(x)                                                                \
  if (x < -32767)                                                              \
    x = -32767;                                                                \
  if (x > 32767)                                                               \
    x = 32767;

#define CONSTRAIN(var, min, max)                                               \
  if (var < (min)) {                                                           \
    var = (min);                                                               \
  } else if (var > (max)) {                                                    \
    var = (max);                                                               \
  }

#endif // MACROS_H_
