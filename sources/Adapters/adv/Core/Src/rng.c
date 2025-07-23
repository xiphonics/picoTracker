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
/* Includes ------------------------------------------------------------------*/
#include "rng.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

RNG_HandleTypeDef hrng;

/* RNG init function */
void MX_RNG_Init(void) {

  /* USER CODE BEGIN RNG_Init 0 */

  /* USER CODE END RNG_Init 0 */

  /* USER CODE BEGIN RNG_Init 1 */

  /* USER CODE END RNG_Init 1 */
  hrng.Instance = RNG;
  hrng.Init.ClockErrorDetection = RNG_CED_ENABLE;
  if (HAL_RNG_Init(&hrng) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN RNG_Init 2 */

  /* USER CODE END RNG_Init 2 */
}

void HAL_RNG_MspInit(RNG_HandleTypeDef *rngHandle) {

  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if (rngHandle->Instance == RNG) {
    /* USER CODE BEGIN RNG_MspInit 0 */

    /* USER CODE END RNG_MspInit 0 */

    /** Initializes the peripherals clock
     */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RNG;
    PeriphClkInitStruct.RngClockSelection = RCC_RNGCLKSOURCE_HSI48;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
      Error_Handler();
    }

    /* RNG clock enable */
    __HAL_RCC_RNG_CLK_ENABLE();
    /* USER CODE BEGIN RNG_MspInit 1 */

    /* USER CODE END RNG_MspInit 1 */
  }
}

void HAL_RNG_MspDeInit(RNG_HandleTypeDef *rngHandle) {

  if (rngHandle->Instance == RNG) {
    /* USER CODE BEGIN RNG_MspDeInit 0 */

    /* USER CODE END RNG_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RNG_CLK_DISABLE();
    /* USER CODE BEGIN RNG_MspDeInit 1 */

    /* USER CODE END RNG_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
