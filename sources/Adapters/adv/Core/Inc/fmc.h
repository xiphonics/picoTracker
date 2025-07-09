/* USER CODE BEGIN Header */
/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2025
 * STMicroelectronics.
 * Copyright (c) 2025 xiphonics, inc.
 *
 * This file is

 * * part of the picoTracker firmware
 */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FMC_H
#define __FMC_H
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern SDRAM_HandleTypeDef hsdram1;
extern SDRAM_HandleTypeDef hsdram2;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_FMC_Init(void);
void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef *hsdram);
void HAL_SDRAM_MspDeInit(SDRAM_HandleTypeDef *hsdram);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__FMC_H */

/**
 * @}
 */

/**
 * @}
 */
