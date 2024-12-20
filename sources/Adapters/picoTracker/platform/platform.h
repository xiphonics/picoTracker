#ifndef _PLATFORM_PICO_H_
#define _PLATFORM_PICO_H_

#include "gpio.h"

#define SD_CONFIG SdioConfig(SDIO_CLK, SDIO_CMD, SDIO_D0)
// TODO: probably not a good idea to allow this in order to avoid malloc
#define FILE_COPY_CONSTRUCTOR_SELECT FILE_COPY_CONSTRUCTOR_PUBLIC

void platform_init();

#endif
