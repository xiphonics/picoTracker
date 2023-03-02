#ifndef SDCARD_H_
#define SDCARD_H_

// SD card driver for SdFat

#ifdef SD_SDIO
class SdioConfig;
extern SdioConfig g_sd_sdio_config;
#define SD_CONFIG g_sd_sdio_config
#else
class SdSpiConfig;
extern SdSpiConfig g_sd_spi_config;
#define SD_CONFIG g_sd_spi_config
#endif

#endif
