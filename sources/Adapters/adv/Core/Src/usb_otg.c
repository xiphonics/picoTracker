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
#include "usb_otg.h"

/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/* USB_OTG_HS init function */

void MX_USB_OTG_HS_USB_Init(void) {

  /* USER CODE BEGIN USB_OTG_HS_Init 0 */

  /* USER CODE END USB_OTG_HS_Init 0 */

  /* USER CODE BEGIN USB_OTG_HS_Init 1 */

  /** Initializes the peripherals clock
   */
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Enable USB Voltage detector
   */
  HAL_PWREx_EnableUSBVoltageDetector();

  /* USB_OTG_HS clock enable */
  __HAL_RCC_USB_OTG_HS_CLK_ENABLE();

  /* USB_OTG_HS interrupt Init */
  // 5 is the maximum interrupt priority allowed by FreeRTOS config
  // (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY) for external calls
  HAL_NVIC_SetPriority(OTG_HS_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(OTG_HS_IRQn);
  /* USER CODE END USB_OTG_HS_Init 1 */
  /* USER CODE BEGIN USB_OTG_HS_Init 2 */
  /* USER CODE END USB_OTG_HS_Init 2 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
