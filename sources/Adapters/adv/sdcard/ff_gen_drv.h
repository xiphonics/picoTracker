/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2023 STMicroelectronics.
 * Copyright (c) 2025 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FF_GEN_DRV_H
#define __FF_GEN_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ff.h"

#include "diskio.h"
#include "stdint.h"

/* Exported types ------------------------------------------------------------*/

/**
 * @brief  Disk IO Driver structure definition
 */
typedef struct {
  DSTATUS (*disk_initialize)(BYTE);                /*!< Initialize Disk Drive*/
  DSTATUS (*disk_status)(BYTE);                    /*!< Get Disk Status*/
  DRESULT (*disk_read)(BYTE, BYTE *, DWORD, UINT); /*!< Read Sector(s)*/
  DRESULT (*disk_write)(BYTE, const BYTE *, DWORD, UINT); /*!< Write Sector(s)*/
  DRESULT (*disk_ioctl)(BYTE, BYTE, void *); /*!< I/O control operation*/
} Diskio_drvTypeDef;

/**
 * @brief  Global Disk IO Drivers structure definition
 */
typedef struct {
  uint8_t is_initialized[FF_VOLUMES];
  const Diskio_drvTypeDef *drv[FF_VOLUMES];
  uint8_t lun[FF_VOLUMES];
  volatile uint8_t nbr;

} Disk_drvTypeDef;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
uint8_t FATFS_LinkDriver(const Diskio_drvTypeDef *drv, char *path);
uint8_t FATFS_UnLinkDriver(char *path);
uint8_t FATFS_LinkDriverEx(const Diskio_drvTypeDef *drv, char *path, BYTE lun);
uint8_t FATFS_UnLinkDriverEx(char *path, BYTE lun);
uint8_t FATFS_GetAttachedDriversNbr(void);

#ifdef __cplusplus
}
#endif

#endif /* __FF_GEN_DRV_H */
