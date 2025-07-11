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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef struct {
  uint32_t magic;   // e.g. 0xBEEFC0DE
  uint32_t length;  // Length of firmware in bytes
  uint32_t version; // Semantic version or build ID
  uint32_t crc32;   // CRC of the firmware (excluding this struct)
  uint32_t type;    // type of firmware
} firmware_info_t;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define SDRAM_BANK_ADDR ((uint32_t)0xD0000000)

#define SDRAM_TIMEOUT ((uint32_t)0xFFFF)
#define REFRESH_COUNT ((uint32_t)0x0603) /* SDRAM refresh counter */

#define SDRAM_MODEREG_BURST_LENGTH_1 ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2 ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4 ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8 ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2 ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3 ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE ((uint16_t)0x0200)

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define CODEC_RESET_Pin GPIO_PIN_13
#define CODEC_RESET_GPIO_Port GPIOC
#define FUEL_GPOUT_Pin GPIO_PIN_14
#define FUEL_GPOUT_GPIO_Port GPIOC
#define CHARGER_PSEL_Pin GPIO_PIN_15
#define CHARGER_PSEL_GPIO_Port GPIOC
#define MIDI_IN_Pin GPIO_PIN_6
#define MIDI_IN_GPIO_Port GPIOF
#define MIDI_OUT_Pin GPIO_PIN_7
#define MIDI_OUT_GPIO_Port GPIOF
#define DISPLAY_PWM_Pin GPIO_PIN_8
#define DISPLAY_PWM_GPIO_Port GPIOF
#define DISPLAY_RST_Pin GPIO_PIN_9
#define DISPLAY_RST_GPIO_Port GPIOF
#define CHARGER_CE_Pin GPIO_PIN_1
#define CHARGER_CE_GPIO_Port GPIOC
#define OSC_EN_Pin GPIO_PIN_2
#define OSC_EN_GPIO_Port GPIOC
#define AMP_EN_Pin GPIO_PIN_3
#define AMP_EN_GPIO_Port GPIOC
#define DISPLAY_SDA_Pin GPIO_PIN_0
#define DISPLAY_SDA_GPIO_Port GPIOA
#define POWER_Pin GPIO_PIN_2
#define POWER_GPIO_Port GPIOA
#define DISPLAY_CS_Pin GPIO_PIN_2
#define DISPLAY_CS_GPIO_Port GPIOB
#define DISPLAY_SCK_Pin GPIO_PIN_13
#define DISPLAY_SCK_GPIO_Port GPIOB
#define INPUT_LEFT_Pin GPIO_PIN_11
#define INPUT_LEFT_GPIO_Port GPIOD
#define INPUT_DOWN_Pin GPIO_PIN_3
#define INPUT_DOWN_GPIO_Port GPIOG
#define SD_DET_Pin GPIO_PIN_11
#define SD_DET_GPIO_Port GPIOA
#define SD_DET_EXTI_IRQn EXTI15_10_IRQn
#define HP_DET_Pin GPIO_PIN_12
#define HP_DET_GPIO_Port GPIOA
#define USB_INT_Pin GPIO_PIN_15
#define USB_INT_GPIO_Port GPIOA
#define INPUT_RIGHT_Pin GPIO_PIN_4
#define INPUT_RIGHT_GPIO_Port GPIOD
#define INPUT_UP_Pin GPIO_PIN_5
#define INPUT_UP_GPIO_Port GPIOD
#define INPUT_ALT_Pin GPIO_PIN_7
#define INPUT_ALT_GPIO_Port GPIOD
#define INPUT_EDIT_Pin GPIO_PIN_9
#define INPUT_EDIT_GPIO_Port GPIOG
#define INPUT_ENTER_Pin GPIO_PIN_11
#define INPUT_ENTER_GPIO_Port GPIOG
#define INPUT_NAV_Pin GPIO_PIN_13
#define INPUT_NAV_GPIO_Port GPIOG
#define INPUT_PLAY_Pin GPIO_PIN_14
#define INPUT_PLAY_GPIO_Port GPIOG
#define CHARGER_OTG_Pin GPIO_PIN_3
#define CHARGER_OTG_GPIO_Port GPIOB
#define CHARGER_INT_Pin GPIO_PIN_7
#define CHARGER_INT_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define DEBUG_UART huart1
#define MIDI_UART huart7
#define I2C hi2c4
#define AUDIO_DMA DMA1_Stream0

// Display setup bitbang
#define DISPLAY_DELAY_CYCLES 4

#define DISPLAY_DELAY                                                          \
  for (uint32_t i = 0; i < DISPLAY_DELAY_CYCLES; i++)                          \
  __NOP()

#define DISPLAY_CS_LOW GPIOB->BSRR = GPIO_BSRR_BR2
#define DISPLAY_CS_HIGH GPIOB->BSRR = GPIO_BSRR_BS2

#define DISPLAY_MOSI_LOW GPIOA->BSRR = GPIO_BSRR_BR0
#define DISPLAY_MOSI_HIGH GPIOA->BSRR = GPIO_BSRR_BS0

#define DISPLAY_SCK_LOW GPIOB->BSRR = GPIO_BSRR_BR13
#define DISPLAY_SCK_HIGH GPIOB->BSRR = GPIO_BSRR_BS13

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
