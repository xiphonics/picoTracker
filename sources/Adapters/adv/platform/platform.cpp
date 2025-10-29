/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "platform.h"
#include "Adapters/adv/mutex/advMutex.h"
#include "System/Console/Trace.h"
#include "tim.h"

void platform_reboot() { NVIC_SystemReset(); };

void platform_bootloader() {

  uint32_t appStack = *(volatile uint32_t *)BOOTLOADER_ADDR;
  uint32_t appResetHandler = *(volatile uint32_t *)(BOOTLOADER_ADDR + 4);
  pFunction appEntry = (pFunction)appResetHandler;

  HAL_DeInit();

  // Disable SysTick (this may not be needed, not using it)
  SysTick->CTRL = 0;
  SysTick->LOAD = 0;
  SysTick->VAL = 0;
  HAL_RCC_DeInit(); // Reset clock configuration

  // We disable TIM8 because that's what we are using as systick
  // 1. Disable TIM8 IRQ at peripheral level
  TIM8->DIER = 0;

  // 2. Stop counter
  TIM8->CR1 &= ~TIM_CR1_CEN;

  // 3. Clear all flags
  TIM8->SR = 0;
  NVIC_DisableIRQ(TIM8_UP_TIM13_IRQn);
  NVIC_ClearPendingIRQ(TIM8_UP_TIM13_IRQn);

  // Disable MPU
  MPU->CTRL = 0;

  // Disable caches
  //  SCB_DisableICache();
  //  SCB_DisableDCache();

  // Memory barrier
  __DSB();
  __ISB();

  // Set vector table offset
  SCB->VTOR = BOOTLOADER_ADDR;

  // Set MSP
  __set_MSP(appStack);

  // Jump to application
  appEntry();
};

SysMutex *platform_mutex() { return new advMutex(); };

uint32_t millis(void) { return __HAL_TIM_GET_COUNTER(&htim2) / 1000; }
uint32_t micros(void) { return __HAL_TIM_GET_COUNTER(&htim2); }

void platform_brightness(uint8_t value) {
  uint8_t min_brightness = 20;

  // Calculate effective range: from min_percent up to 100%
  float effective_range = 100.0f - min_brightness;

  // Normalize value (0–255) to a 0.0–1.0 range
  float normalized = (float)value / 255.0f;

  // Final percentage to set
  float pwm_percent = min_brightness + normalized * effective_range;

  // Convert to timer pulse (assuming Period = 999 for 100%)
  uint32_t pulse = (uint32_t)((pwm_percent / 100.0f) *
                              (__HAL_TIM_GET_AUTORELOAD(&htim13) + 1));

  __HAL_TIM_SET_COMPARE(&htim13, TIM_CHANNEL_1, pulse);
}
