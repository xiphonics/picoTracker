// Driver and interface for accessing SD card in SPI mode

#include "Adapters/picoTracker/platform/platform.h"
#include "System/Console/Trace.h"
#include <SdFat.h>
#include <cstdio>
#include <hardware/gpio.h>
#include <hardware/spi.h>

#ifndef SD_SDIO

class RP2040SPIDriver : public SdSpiBaseClass {
public:
  void begin(SdSpiConfig config) {
    // Set direction
    gpio_set_dir(SD_SPI_SCK, true);
    gpio_set_dir(SD_SPI_MOSI, true);
    gpio_set_dir(SD_SPI_MISO, false);
    gpio_set_dir(SD_SPI_CS, true);

    // Set pullups
    gpio_pull_up(SD_SPI_SCK);
    gpio_pull_up(SD_SPI_MOSI);
    gpio_pull_up(SD_SPI_MISO);
    gpio_pull_up(SD_SPI_CS);

    // Set funcs
    gpio_set_function(SD_SPI_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SD_SPI_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(SD_SPI_MISO, GPIO_FUNC_SPI);
    gpio_set_function(SD_SPI_CS, GPIO_FUNC_SIO);
  }

  void activate() {
    #if SHOW_SPI_DEBUG
    uint baudrate = spi_init(SD_SPI, m_sckfreq);
    Trace::Log("SDCARD", "SD SPI baudrate: %i\n", baudrate);
    #else
    spi_init(SD_SPI, m_sckfreq);
    #endif
    spi_set_format(SD_SPI, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
  }

  void deactivate() {}

  void wait_idle() {
    while (!(spi_get_hw(SD_SPI)->sr & SPI_SSPSR_TFE_BITS))
      ;
    while (spi_get_hw(SD_SPI)->sr & SPI_SSPSR_BSY_BITS)
      ;
  }

  // Single byte receive
  uint8_t receive() {
    uint8_t tx = 0xFF;
    uint8_t rx;
    spi_write_read_blocking(SD_SPI, &tx, &rx, 1);
    return rx;
  }

  // Single byte send
  void send(uint8_t data) {
    spi_write_blocking(SD_SPI, &data, 1);
    wait_idle();
  }

  // Multiple byte receive
  uint8_t receive(uint8_t *buf, size_t count) {
    spi_read_blocking(SD_SPI, 0xFF, buf, count);
    return 0;
  }

  // Multiple byte send
  void send(const uint8_t *buf, size_t count) {
    spi_write_blocking(SD_SPI, buf, count);
  }

  void setSckSpeed(uint32_t maxSck) { m_sckfreq = maxSck; }

private:
  uint32_t m_sckfreq;
};

void sdCsInit(SdCsPin_t pin) {}

void sdCsWrite(SdCsPin_t pin, bool level) {
  if (level)
    sio_hw->gpio_set = (1 << SD_SPI_CS);
  else
    sio_hw->gpio_clr = (1 << SD_SPI_CS);
}

RP2040SPIDriver g_sd_spi_port;
SdSpiConfig g_sd_spi_config(0, DEDICATED_SPI, SD_SCK_MHZ(24), &g_sd_spi_port);

#endif
