/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2023 STMicroelectronics.
 * Copyright (c) 2025 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SD_DISKIO_DMA_STANDALONE_H
#define __SD_DISKIO_DMA_STANDALONE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/*
 * The sd_diskio_config.h is under application project
 * and contain the config parameters for SD diskio
 *
 */
#include "ff_gen_drv.h"
#include "sd_diskio_config.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern const Diskio_drvTypeDef SD_DMA_Driver;

#ifdef __cplusplus
}
#endif

#endif /* __SD_DISKIO_DMA_STANDALONE_H */
