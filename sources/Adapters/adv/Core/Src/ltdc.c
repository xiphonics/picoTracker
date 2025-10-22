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
#include "ltdc.h"

/* USER CODE BEGIN 0 */
#include <string.h>
__attribute__((section(".FRAMEBUFFER"))) __attribute__((aligned(32)))
uint16_t framebuffer[DISPLAY_WIDTH * DISPLAY_HEIGHT];
__attribute__((section(".FRAMEBUFFER"))) __attribute__((aligned(32)))
uint16_t placeholder_framebuffer_layer2[DISPLAY_WIDTH * DISPLAY_HEIGHT];

void init_display(void);
/* USER CODE END 0 */

LTDC_HandleTypeDef hltdc;

/* LTDC init function */
void MX_LTDC_Init(void) {

  /* USER CODE BEGIN LTDC_Init 0 */

  /* USER CODE END LTDC_Init 0 */

  LTDC_LayerCfgTypeDef pLayerCfg = {0};

  /* USER CODE BEGIN LTDC_Init 1 */

  /* USER CODE END LTDC_Init 1 */
  hltdc.Instance = LTDC;
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc.Init.HorizontalSync = 4;
  hltdc.Init.VerticalSync = 4;
  hltdc.Init.AccumulatedHBP = 84;
  hltdc.Init.AccumulatedVBP = 24;
  hltdc.Init.AccumulatedActiveW = 804;
  hltdc.Init.AccumulatedActiveH = 744;
  hltdc.Init.TotalWidth = 819;
  hltdc.Init.TotalHeigh = 759;
  hltdc.Init.Backcolor.Blue = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Red = 0;
  if (HAL_LTDC_Init(&hltdc) != HAL_OK) {
    Error_Handler();
  }
  pLayerCfg.WindowX0 = 0;
  pLayerCfg.WindowX1 = 720;
  pLayerCfg.WindowY0 = 0;
  pLayerCfg.WindowY1 = 720;
  pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
  pLayerCfg.Alpha = 255;
  pLayerCfg.Alpha0 = 0;
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
  pLayerCfg.FBStartAdress = 0;
  pLayerCfg.ImageWidth = 720;
  pLayerCfg.ImageHeight = 720;
  pLayerCfg.Backcolor.Blue = 0;
  pLayerCfg.Backcolor.Green = 0;
  pLayerCfg.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN LTDC_Init 2 */
  // clear framebuffer
  memset(framebuffer, 0, sizeof(framebuffer));
  pLayerCfg.FBStartAdress = (uint32_t)framebuffer;
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK) {
    Error_Handler();
  }

  // init the display
  init_display();
  /* USER CODE END LTDC_Init 2 */
}

void HAL_LTDC_MspInit(LTDC_HandleTypeDef *ltdcHandle) {

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if (ltdcHandle->Instance == LTDC) {
    /* USER CODE BEGIN LTDC_MspInit 0 */

    /* USER CODE END LTDC_MspInit 0 */

    /** Initializes the peripherals clock
     */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
    PeriphClkInitStruct.PLL3.PLL3M = 4;
    PeriphClkInitStruct.PLL3.PLL3N = 185;
    PeriphClkInitStruct.PLL3.PLL3P = 32;
    PeriphClkInitStruct.PLL3.PLL3Q = 32;
    PeriphClkInitStruct.PLL3.PLL3R = 28;
    PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_1;
    PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
    PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
      Error_Handler();
    }

    /* LTDC clock enable */
    __HAL_RCC_LTDC_CLK_ENABLE();

    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**LTDC GPIO Configuration
    PF10     ------> LTDC_DE
    PC0     ------> LTDC_R5
    PA1     ------> LTDC_R2
    PA3     ------> LTDC_B5
    PA4     ------> LTDC_VSYNC
    PA5     ------> LTDC_R4
    PA6     ------> LTDC_G2
    PB0     ------> LTDC_R3
    PB1     ------> LTDC_R6
    PB10     ------> LTDC_G4
    PB11     ------> LTDC_G5
    PG6     ------> LTDC_R7
    PG7     ------> LTDC_CLK
    PC6     ------> LTDC_HSYNC
    PC7     ------> LTDC_G6
    PA8     ------> LTDC_B3
    PD3     ------> LTDC_G7
    PD6     ------> LTDC_B2
    PG10     ------> LTDC_G3
    PG12     ------> LTDC_B4
    PB8     ------> LTDC_B6
    PB9     ------> LTDC_B7
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin =
        GPIO_PIN_1 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_LTDC;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF13_LTDC;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_LTDC;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    /* USER CODE BEGIN LTDC_MspInit 1 */

    /* USER CODE END LTDC_MspInit 1 */
  }
}

void HAL_LTDC_MspDeInit(LTDC_HandleTypeDef *ltdcHandle) {

  if (ltdcHandle->Instance == LTDC) {
    /* USER CODE BEGIN LTDC_MspDeInit 0 */

    /* USER CODE END LTDC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_LTDC_CLK_DISABLE();

    /**LTDC GPIO Configuration
    PF10     ------> LTDC_DE
    PC0     ------> LTDC_R5
    PA1     ------> LTDC_R2
    PA3     ------> LTDC_B5
    PA4     ------> LTDC_VSYNC
    PA5     ------> LTDC_R4
    PA6     ------> LTDC_G2
    PB0     ------> LTDC_R3
    PB1     ------> LTDC_R6
    PB10     ------> LTDC_G4
    PB11     ------> LTDC_G5
    PG6     ------> LTDC_R7
    PG7     ------> LTDC_CLK
    PC6     ------> LTDC_HSYNC
    PC7     ------> LTDC_G6
    PA8     ------> LTDC_B3
    PD3     ------> LTDC_G7
    PD6     ------> LTDC_B2
    PG10     ------> LTDC_G3
    PG12     ------> LTDC_B4
    PB8     ------> LTDC_B6
    PB9     ------> LTDC_B7
    */
    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_10);

    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0 | GPIO_PIN_6 | GPIO_PIN_7);

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 |
                               GPIO_PIN_6 | GPIO_PIN_8);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_10 | GPIO_PIN_11 |
                               GPIO_PIN_8 | GPIO_PIN_9);

    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_10 | GPIO_PIN_12);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_3 | GPIO_PIN_6);

    /* USER CODE BEGIN LTDC_MspDeInit 1 */

    /* USER CODE END LTDC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
// BIT BANG
// RGB+9b_SPI(rise)
void SPI_SendData(unsigned char i) {
  unsigned char n;

  for (n = 0; n < 8; n++) {
    DISPLAY_SCK_LOW;
    DISPLAY_DELAY;
    if (i & 0x80)
      DISPLAY_MOSI_HIGH;
    else
      DISPLAY_MOSI_LOW;
    DISPLAY_SCK_HIGH;
    i <<= 1;
    DISPLAY_DELAY;
  }
}
void SPI_WriteComm(unsigned char i) {
  DISPLAY_CS_LOW;
  DISPLAY_DELAY;
  DISPLAY_MOSI_LOW;
  DISPLAY_DELAY;
  DISPLAY_SCK_LOW;
  DISPLAY_DELAY;
  DISPLAY_SCK_HIGH;
  DISPLAY_DELAY;
  SPI_SendData(i);
  DISPLAY_CS_HIGH;
}

void SPI_WriteData(unsigned char i) {
  DISPLAY_CS_LOW;
  DISPLAY_DELAY;
  DISPLAY_MOSI_HIGH;
  DISPLAY_DELAY;
  DISPLAY_SCK_LOW;
  DISPLAY_DELAY;
  DISPLAY_SCK_HIGH;
  DISPLAY_DELAY;
  SPI_SendData(i);
  DISPLAY_CS_HIGH;
}

void init_display(void) {
  //  Trace::Log("INFO", "Display initialization...");

  // Hardware Reset Sequence
  HAL_GPIO_WritePin(DISPLAY_RST_GPIO_Port, DISPLAY_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(10); // Minimum reset time
  HAL_GPIO_WritePin(DISPLAY_RST_GPIO_Port, DISPLAY_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(10); // Minimum reset time
  HAL_GPIO_WritePin(DISPLAY_RST_GPIO_Port, DISPLAY_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(200); // Display requires ~120ms after reset

  // 720X720 BOE3.95 6G ��B3 QV040YNQ-N80��
  SPI_WriteComm(0xFF);
  SPI_WriteData(0x30);
  SPI_WriteComm(0xFF);
  SPI_WriteData(0x52);
  SPI_WriteComm(0xFF);
  SPI_WriteData(0x01); // Enter page 1 commands
  SPI_WriteComm(0xE3);
  SPI_WriteData(0x00); // ???
  SPI_WriteComm(0x0A);
  SPI_WriteData(0x00); // WRMADC_EN - Enable MADCTL
  SPI_WriteComm(0x23);
  SPI_WriteData(0x20); // RGB interface control - sync mode
  // DE Mode - VS polarity: low, HS polarity low, PCLK polarity: low, DE
  // polarity high enable
  SPI_WriteComm(0x24);
  SPI_WriteData(0x0f); // ???
  SPI_WriteComm(0x25);
  SPI_WriteData(0x14); // ???
  SPI_WriteComm(0x26);
  SPI_WriteData(0x2E); // ???
  SPI_WriteComm(0x27);
  SPI_WriteData(0x2E); // ???
  SPI_WriteComm(0x29);
  SPI_WriteData(0x02); // ???
  SPI_WriteComm(0x2A);
  SPI_WriteData(0xCF); // ???
  SPI_WriteComm(0x32);
  SPI_WriteData(0x34); // ???
  SPI_WriteComm(0x38);
  SPI_WriteData(0x9C); // vap_adj - 4.605
  SPI_WriteComm(0x39);
  SPI_WriteData(0xA7); // van_adj - -4.6
  SPI_WriteComm(0x3A);
  SPI_WriteData(0x77); // vcom_adj - -1.6750
  SPI_WriteComm(0x3B);
  SPI_WriteData(0x94); // ??
  SPI_WriteComm(0x40);
  SPI_WriteData(0x07); // ??
  SPI_WriteComm(0x42);
  SPI_WriteData(0x6D); // ??
  SPI_WriteComm(0x43);
  SPI_WriteData(0x83); // ??
  SPI_WriteComm(0x81);
  SPI_WriteData(0x00); // BOOST_CTRL2 - many things
  SPI_WriteComm(0x91);
  SPI_WriteData(0x57); // EXTPW_CTRL2 -
  SPI_WriteComm(0x92);
  SPI_WriteData(0x57); // EXTPW_CTRL3 -
  SPI_WriteComm(0xA0);
  SPI_WriteData(0x52); // ??? down to
  SPI_WriteComm(0xA1);
  SPI_WriteData(0x50);
  SPI_WriteComm(0xA4);
  SPI_WriteData(0x9C);
  SPI_WriteComm(0xA7);
  SPI_WriteData(0x02);
  SPI_WriteComm(0xA8);
  SPI_WriteData(0x02);
  SPI_WriteComm(0xA9);
  SPI_WriteData(0x02);
  SPI_WriteComm(0xAA);
  SPI_WriteData(0xA8);
  SPI_WriteComm(0xAB);
  SPI_WriteData(0x28);
  SPI_WriteComm(0xAE);
  SPI_WriteData(0xD2);
  SPI_WriteComm(0xAF);
  SPI_WriteData(0x02);
  SPI_WriteComm(0xB0);
  SPI_WriteData(0xD2);
  SPI_WriteComm(0xB2);
  SPI_WriteData(0x26);
  SPI_WriteComm(0xB3);
  SPI_WriteData(0x26); // ???

  SPI_WriteComm(0xFF);
  SPI_WriteData(0x30);
  SPI_WriteComm(0xFF);
  SPI_WriteData(0x52);
  SPI_WriteComm(0xFF);
  SPI_WriteData(0x02); // Select page 2

  SPI_WriteComm(0xB0);
  SPI_WriteData(0x02); // PGAMVR0
  SPI_WriteComm(0xB1);
  SPI_WriteData(0x0E); // PGAMVR1
  SPI_WriteComm(0xB2);
  SPI_WriteData(0x08); // PGAMVR2
  SPI_WriteComm(0xB3);
  SPI_WriteData(0x29); // PGAMVR3
  SPI_WriteComm(0xB4);
  SPI_WriteData(0x28); // PGAMVR4
  SPI_WriteComm(0xB5);
  SPI_WriteData(0x37); // PGAMVR5
  SPI_WriteComm(0xB6);
  SPI_WriteData(0x12); // PGAMPR0
  SPI_WriteComm(0xB7);
  SPI_WriteData(0x32); // PGAMPR1
  SPI_WriteComm(0xB8);
  SPI_WriteData(0x0B); // PGAMPK0
  SPI_WriteComm(0xB9);
  SPI_WriteData(0x03); // PGAMPK1
  SPI_WriteComm(0xBA);
  SPI_WriteData(0x0E); // PGAMPK2
  SPI_WriteComm(0xBB);
  SPI_WriteData(0x0D); // PGAMPK3
  SPI_WriteComm(0xBC);
  SPI_WriteData(0x10); // PGAMPK4
  SPI_WriteComm(0xBD);
  SPI_WriteData(0x13); // PGAMPK5
  SPI_WriteComm(0xBE);
  SPI_WriteData(0x18); // PGAMPK6
  SPI_WriteComm(0xBF);
  SPI_WriteData(0x0F); // PGAMPK7
  SPI_WriteComm(0xC0);
  SPI_WriteData(0x16); // PGAMPK8
  SPI_WriteComm(0xC1);
  SPI_WriteData(0x08); // PGAMPK9

  SPI_WriteComm(0xD0);
  SPI_WriteData(0x05); // NGAMVR0
  SPI_WriteComm(0xD1);
  SPI_WriteData(0x0B); // NGAMVR1
  SPI_WriteComm(0xD2);
  SPI_WriteData(0x03); // NGAMVR2
  SPI_WriteComm(0xD3);
  SPI_WriteData(0x33); // NGAMVR3
  SPI_WriteComm(0xD4);
  SPI_WriteData(0x32); // NGAMVR4
  SPI_WriteComm(0xD5);
  SPI_WriteData(0x32); // NGAMVR5
  SPI_WriteComm(0xD6);
  SPI_WriteData(0x0F); // NGAMPR0
  SPI_WriteComm(0xD7);
  SPI_WriteData(0x39); // NGAMPR1
  SPI_WriteComm(0xD8);
  SPI_WriteData(0x0B); // NGAMPK0
  SPI_WriteComm(0xD9);
  SPI_WriteData(0x02); // NGAMPK1
  SPI_WriteComm(0xDA);
  SPI_WriteData(0x10); // NGAMPK2
  SPI_WriteComm(0xDB);
  SPI_WriteData(0x0F); // NGAMPK3
  SPI_WriteComm(0xDC);
  SPI_WriteData(0x11); // NGAMPK4
  SPI_WriteComm(0xDD);
  SPI_WriteData(0x14); // NGAMPK5
  SPI_WriteComm(0xDE);
  SPI_WriteData(0x1A); // NGAMPK6
  SPI_WriteComm(0xDF);
  SPI_WriteData(0x11); // NGAMPK7
  SPI_WriteComm(0xE0);
  SPI_WriteData(0x18); // NGAMPK8
  SPI_WriteComm(0xE1);
  SPI_WriteData(0x04); // NGAMPK9

  SPI_WriteComm(0xFF);
  SPI_WriteData(0x30);
  SPI_WriteComm(0xFF);
  SPI_WriteData(0x52);
  SPI_WriteComm(0xFF);
  SPI_WriteData(0x03); // Page 3
  SPI_WriteComm(0x00);
  SPI_WriteData(0x00); // GIP_VST_1
  SPI_WriteComm(0x01);
  SPI_WriteData(0x00); // GIP_VST_2
  SPI_WriteComm(0x02);
  SPI_WriteData(0x00); // GIP_VST_3
  SPI_WriteComm(0x03);
  SPI_WriteData(0x00); // GIP_VST_4
  SPI_WriteComm(0x08);
  SPI_WriteData(0x0D); // GIP_VST_9
  SPI_WriteComm(0x09);
  SPI_WriteData(0x0E); // GIP_VST_10
  SPI_WriteComm(0x0A);
  SPI_WriteData(0x0F); // GIP_VST_11
  SPI_WriteComm(0x0B);
  SPI_WriteData(0x10); // GIP_VST_12
  SPI_WriteComm(0x20);
  SPI_WriteData(0x00); // GIP_VEND_1
  SPI_WriteComm(0x21);
  SPI_WriteData(0x00); // GIP_VEND_2
  SPI_WriteComm(0x22);
  SPI_WriteData(0x00); // GIP_VEND_3
  SPI_WriteComm(0x23);
  SPI_WriteData(0x00); // GIP_VEND_4
  SPI_WriteComm(0x28);
  SPI_WriteData(0x22); // GIP_VEND_9
  SPI_WriteComm(0x2A);
  SPI_WriteData(0xE9); // GIP_VEND_11
  SPI_WriteComm(0x2B);
  SPI_WriteData(0xE9); // GIP_VEND_12
  SPI_WriteComm(0x30);
  SPI_WriteData(0x00); // GIP_CLK_1
  SPI_WriteComm(0x31);
  SPI_WriteData(0x00); // GIP_CLK_2
  SPI_WriteComm(0x32);
  SPI_WriteData(0x00); // GIP_CLK_3
  SPI_WriteComm(0x33);
  SPI_WriteData(0x00); // GIP_CLK_4
  SPI_WriteComm(0x34);
  SPI_WriteData(0x01); // GIP_CLK_5
  SPI_WriteComm(0x35);
  SPI_WriteData(0x00); // GIP_CLK_6
  SPI_WriteComm(0x36);
  SPI_WriteData(0x00); // GIP_CLK_7
  SPI_WriteComm(0x37);
  SPI_WriteData(0x03); // GIP_CLK_8
  SPI_WriteComm(0x40);
  SPI_WriteData(0x0A); // GIP_CLKA_1
  SPI_WriteComm(0x41);
  SPI_WriteData(0x0B); // GIP_CLKA_2
  SPI_WriteComm(0x42);
  SPI_WriteData(0x0C); // GIP_CLKA_3
  SPI_WriteComm(0x43);
  SPI_WriteData(0x0D); // GIP_CLKA_4
  SPI_WriteComm(0x44);
  SPI_WriteData(0x22); // GIP_CLKA_5
  SPI_WriteComm(0x45);
  SPI_WriteData(0xE4); // GIP_CLKA_6
  SPI_WriteComm(0x46);
  SPI_WriteData(0xE5); // GIP_CLKA_7
  SPI_WriteComm(0x47);
  SPI_WriteData(0x22); // GIP_CLKA_8
  SPI_WriteComm(0x48);
  SPI_WriteData(0xE6); // GIP_CLKA_9
  SPI_WriteComm(0x49);
  SPI_WriteData(0xE7); // GIP_CLKA_10
  SPI_WriteComm(0x50);
  SPI_WriteData(0x0E); // GIP_CLKB_1
  SPI_WriteComm(0x51);
  SPI_WriteData(0x0F); // GIP_CLKB_2
  SPI_WriteComm(0x52);
  SPI_WriteData(0x10); // GIP_CLKB_3
  SPI_WriteComm(0x53);
  SPI_WriteData(0x11); // GIP_CLKB_4

  SPI_WriteComm(0x54);
  SPI_WriteData(0x22); // GIP_CLKB_5
  SPI_WriteComm(0x55);
  SPI_WriteData(0xE8); // GIP_CLKB_6
  SPI_WriteComm(0x56);
  SPI_WriteData(0xE9); // GIP_CLKB_7
  SPI_WriteComm(0x57);
  SPI_WriteData(0x22); // GIP_CLKB_8
  SPI_WriteComm(0x58);
  SPI_WriteData(0xEA); // GIP_CLKB_9
  SPI_WriteComm(0x59);
  SPI_WriteData(0xEB); // GIP_CLKB_10

  SPI_WriteComm(0x60);
  SPI_WriteData(0x05); // GIP_CLKC_1
  SPI_WriteComm(0x61);
  SPI_WriteData(0x05); // GIP_CLKC_2
  SPI_WriteComm(0x65);
  SPI_WriteData(0x0A); // GIP_CLKC_6
  SPI_WriteComm(0x66);
  SPI_WriteData(0x0A); // GIP_CLKC_7
  SPI_WriteComm(0x80);
  SPI_WriteData(0x05); // PANELU2D1
  SPI_WriteComm(0x81);
  SPI_WriteData(0x00); // PANELU2D2
  SPI_WriteComm(0x82);
  SPI_WriteData(0x02); // PANELU2D3
  SPI_WriteComm(0x83);
  SPI_WriteData(0x04); // PANELU2D4
  SPI_WriteComm(0x84);
  SPI_WriteData(0x00); // PANELU2D5
  SPI_WriteComm(0x85);
  SPI_WriteData(0x00); // PANELU2D6
  SPI_WriteComm(0x86);
  SPI_WriteData(0x1f); // PANELU2D7
  SPI_WriteComm(0x87);
  SPI_WriteData(0x1f); // PANELU2D8
  SPI_WriteComm(0x88);
  SPI_WriteData(0x0a); // PANELU2D9
  SPI_WriteComm(0x89);
  SPI_WriteData(0x0c); // PANELU2D10
  SPI_WriteComm(0x8A);
  SPI_WriteData(0x0e); // PANELU2D11
  SPI_WriteComm(0x8B);
  SPI_WriteData(0x10); // PANELU2D12

  SPI_WriteComm(0x96);
  SPI_WriteData(0x05); // PANELU2D23
  SPI_WriteComm(0x97);
  SPI_WriteData(0x00); // PANELU2D24
  SPI_WriteComm(0x98);
  SPI_WriteData(0x01); // PANELU2D25
  SPI_WriteComm(0x99);
  SPI_WriteData(0x03); // PANELU2D26
  SPI_WriteComm(0x9A);
  SPI_WriteData(0x00); // PANELU2D27
  SPI_WriteComm(0x9B);
  SPI_WriteData(0x00); // PANELU2D28
  SPI_WriteComm(0x9C);
  SPI_WriteData(0x1f); // PANELU2D29
  SPI_WriteComm(0x9D);
  SPI_WriteData(0x1f); // PANELU2D30
  SPI_WriteComm(0x9E);
  SPI_WriteData(0x09); // PANELU2D31
  SPI_WriteComm(0x9F);
  SPI_WriteData(0x0b); // PANELU2D32
  SPI_WriteComm(0xA0);
  SPI_WriteData(0x0d); // PANELU2D33
  SPI_WriteComm(0xA1);
  SPI_WriteData(0x0f); // PANELU2D34

  SPI_WriteComm(0xFF);
  SPI_WriteData(0x30);
  SPI_WriteComm(0xFF);
  SPI_WriteData(0x52);
  SPI_WriteComm(0xFF);
  SPI_WriteData(0x00); // page 00
  //      SPI_WriteComm(0x3A);SPI_WriteData(0x66);
  SPI_WriteComm(0x0C);
  SPI_WriteData(
      0x66); // Read Display Pixel Format ???? should be 0x60? lower ignored?

  SPI_WriteComm(0x36);
  SPI_WriteData(0x0A); // Display Access Control - flips

  SPI_WriteComm(0x11);
  SPI_WriteData(0x00); // Sleep Out
  HAL_Delay(200);
  SPI_WriteComm(0x29);
  SPI_WriteData(0x00); // Display On
  HAL_Delay(10);
  //  Trace::Log("INFO", "Display initialized...");
}
/* USER CODE END 1 */
