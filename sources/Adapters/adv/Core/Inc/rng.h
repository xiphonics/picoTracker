/* USER CODE BEGIN Header */
/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2025
 * STMicroelectronics.
 * Copyright (c) 2025 xiphonics, inc.
 *
 * This file is
 * part of the picoTracker firmware
 */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RNG_H__
#define __RNG_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern RNG_HandleTypeDef hrng;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_RNG_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __RNG_H__ */
