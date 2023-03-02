#ifndef _PICO_UTIL_H_
#define _PICO_UTIL_H_

#include "pico/stdlib.h"

uint32_t millis(void);
uint32_t micros(void);
uint32_t measure_free_mem(void);
void measure_freqs(void);

#endif
