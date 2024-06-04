// Driver for accessing SD card in SDIO mode on RP2040.

#include "System/Console/Trace.h"
#include "sdio.h"
#include <SdCard/SdCardInfo.h>
#include <SdFat.h>
#include <hardware/gpio.h>

static uint32_t g_sdio_ocr; // Operating condition register from card
static uint32_t g_sdio_rca; // Relative card address
static cid_t g_sdio_cid;
static csd_t g_sdio_csd;
static int g_sdio_error_line;
static sdio_status_t g_sdio_error;
static uint32_t g_sdio_dma_buf[128];
static uint32_t g_sdio_sector_count;

#define checkReturnOk(call)                                                    \
  ((g_sdio_error = (call)) == SDIO_OK ? true : logSDError(__LINE__))
static bool logSDError(int line) {
  g_sdio_error_line = line;
  Trace::Log("SDIO", "SDIO SD card error on line ", line, ", error code ",
             (int)g_sdio_error);
  return false;
}

// Callback used by SCSI code for simultaneous processing
static sd_callback_t m_stream_callback;
static const uint8_t *m_stream_buffer;
static uint32_t m_stream_count;
static uint32_t m_stream_count_start;

void azplatform_set_sd_callback(sd_callback_t func, const uint8_t *buffer) {
  m_stream_callback = func;
  m_stream_buffer = buffer;
  m_stream_count = 0;
  m_stream_count_start = 0;
}

static sd_callback_t get_stream_callback(const uint8_t *buf, uint32_t count,
                                         const char *accesstype,
                                         uint32_t sector) {
  m_stream_count_start = m_stream_count;

  if (m_stream_callback) {
    if (buf == m_stream_buffer + m_stream_count) {
      m_stream_count += count;
      return m_stream_callback;
    } else {
      Trace::Debug("SD card ", accesstype, "(", (int)sector,
                   ") slow transfer, buffer", (uint32_t)buf, " vs. ",
                   (uint32_t)(m_stream_buffer + m_stream_count));
      return NULL;
    }
  }

  return NULL;
}

bool SdioCard::begin(SdioConfig sdioConfig) {
  uint32_t reply;
  sdio_status_t status;

  // Initialize at 1 MHz clock speed
  rp2040_sdio_init(25);

  // Establish initial connection with the card
  for (int retries = 0; retries < 5; retries++) {
    sleep_ms(1);
    reply = 0;
    rp2040_sdio_command_R1(CMD0, 0, NULL);                // GO_IDLE_STATE
    status = rp2040_sdio_command_R1(CMD8, 0x1AA, &reply); // SEND_IF_COND

    if (status == SDIO_OK && reply == 0x1AA) {
      break;
    }
  }

  if (reply != 0x1AA || status != SDIO_OK) {
    // Trace::Debug("SDIO not responding to CMD8 SEND_IF_COND, status ",
    // (int)status, " reply ", reply);
    return false;
  }

  // Send ACMD41 to begin card initialization and wait for it to complete
  uint32_t start = millis();
  do {
    if (!checkReturnOk(rp2040_sdio_command_R1(CMD55, 0, &reply)) || // APP_CMD
        !checkReturnOk(rp2040_sdio_command_R3(ACMD41, 0xD0040000,
                                              &g_sdio_ocr))) // 3.0V voltage
    // !checkReturnOk(rp2040_sdio_command_R1(ACMD41, 0xC0100000, &g_sdio_ocr)))
    {
      return false;
    }

    if ((uint32_t)(millis() - start) > 1000) {
      Trace::Log("SDIO", "card initialization timeout");
      return false;
    }
  } while (!(g_sdio_ocr & (1 << 31)));

  // Get CID
  if (!checkReturnOk(rp2040_sdio_command_R2(CMD2, 0, (uint8_t *)&g_sdio_cid))) {
    Trace::Debug("SDIO failed to read CID");
    return false;
  }

  // Get relative card address
  if (!checkReturnOk(rp2040_sdio_command_R1(CMD3, 0, &g_sdio_rca))) {
    Trace::Debug("SDIO failed to get RCA");
    return false;
  }

  // Get CSD
  if (!checkReturnOk(
          rp2040_sdio_command_R2(CMD9, g_sdio_rca, (uint8_t *)&g_sdio_csd))) {
    Trace::Debug("SDIO failed to read CSD");
    return false;
  }

  g_sdio_sector_count = sectorCount();

  // Select card
  if (!checkReturnOk(rp2040_sdio_command_R1(CMD7, g_sdio_rca, &reply))) {
    Trace::Debug("SDIO failed to select card");
    return false;
  }

  // Set 4-bit bus mode
  if (!checkReturnOk(rp2040_sdio_command_R1(CMD55, g_sdio_rca, &reply)) ||
      !checkReturnOk(rp2040_sdio_command_R1(ACMD6, 2, &reply))) {
    Trace::Debug("SDIO failed to set bus width");
    return false;
  }

  // Increase to 25 MHz clock rate
  rp2040_sdio_init(1);

  return true;
}

uint8_t SdioCard::errorCode() const { return g_sdio_error; }

uint32_t SdioCard::errorData() const { return 0; }

uint32_t SdioCard::errorLine() const { return g_sdio_error_line; }

bool SdioCard::isBusy() { return (sio_hw->gpio_in & (1 << SDIO_D0)) == 0; }

uint32_t SdioCard::kHzSdClk() { return 0; }

bool SdioCard::readCID(cid_t *cid) {
  *cid = g_sdio_cid;
  return true;
}

bool SdioCard::readCSD(csd_t *csd) {
  *csd = g_sdio_csd;
  return true;
}

bool SdioCard::readOCR(uint32_t *ocr) {
  // SDIO mode does not have CMD58, but main program uses this to
  // poll for card presence. Return status register instead.
  return checkReturnOk(rp2040_sdio_command_R1(CMD13, g_sdio_rca, ocr));
}

bool SdioCard::readData(uint8_t *dst) {
  Trace::Log("SDIO", "SdioCard::readData() called but not implemented!");
  return false;
}

bool SdioCard::readStart(uint32_t sector) {
  Trace::Log("SDIO", "SdioCard::readStart() called but not implemented!");
  return false;
}

bool SdioCard::readStop() {
  Trace::Log("SDIO", "SdioCard::readStop() called but not implemented!");
  return false;
}

uint32_t SdioCard::sectorCount() { return g_sdio_csd.capacity(); }

uint32_t SdioCard::status() {
  uint32_t reply;
  if (checkReturnOk(rp2040_sdio_command_R1(CMD13, g_sdio_rca, &reply)))
    return reply;
  else
    return 0;
}

bool SdioCard::stopTransmission(bool blocking) {
  uint32_t reply;
  if (!checkReturnOk(rp2040_sdio_command_R1(CMD12, 0, &reply))) {
    return false;
  }

  if (!blocking) {
    return true;
  } else {
    uint32_t end = millis() + 100;
    while (millis() < end && isBusy()) {
      if (m_stream_callback) {
        m_stream_callback(m_stream_count);
      }
    }
    if (isBusy()) {
      Trace::Log("SDIO", "SdioCard::stopTransmission() timeout");
      return false;
    } else {
      return true;
    }
  }
}

bool SdioCard::syncDevice() { return true; }

uint8_t SdioCard::type() const {
  if (g_sdio_ocr & (1 << 30))
    return SD_CARD_TYPE_SDHC;
  else
    return SD_CARD_TYPE_SD2;
}

bool SdioCard::writeData(const uint8_t *src) {
  Trace::Log("SDIO", "SdioCard::writeData() called but not implemented!");
  return false;
}

bool SdioCard::writeStart(uint32_t sector) {
  Trace::Log("SDIO", "SdioCard::writeStart() called but not implemented!");
  return false;
}

bool SdioCard::writeStop() {
  Trace::Log("SDIO", "SdioCard::writeStop() called but not implemented!");
  return false;
}

bool SdioCard::erase(uint32_t firstSector, uint32_t lastSector) {
  Trace::Log("SDIO", "SdioCard::erase() not implemented");
  return false;
}

bool SdioCard::cardCMD6(uint32_t arg, uint8_t *status) {
  Trace::Log("SDIO", "SdioCard::cardCMD6() not implemented");
  return false;
}

bool SdioCard::readSCR(scr_t *scr) {
  Trace::Log("SDIO", "SdioCard::readSCR() not implemented");
  return false;
}

/* Writing and reading, with progress callback */

bool SdioCard::writeSector(uint32_t sector, const uint8_t *src) {
  if (((uint32_t)src & 3) != 0) {
    // Buffer is not aligned, need to memcpy() the data to a temporary buffer.
    memcpy(g_sdio_dma_buf, src, sizeof(g_sdio_dma_buf));
    src = (uint8_t *)g_sdio_dma_buf;
  }

  // If possible, report transfer status to application through callback.
  sd_callback_t callback = get_stream_callback(src, 512, "writeSector", sector);

  uint32_t reply;
  if (!checkReturnOk(rp2040_sdio_command_R1(16, 512, &reply)) || // SET_BLOCKLEN
      !checkReturnOk(
          rp2040_sdio_command_R1(CMD24, sector, &reply)) || // WRITE_BLOCK
      !checkReturnOk(rp2040_sdio_tx_start(src, 1))) // Start transmission
  {
    return false;
  }

  do {
    uint32_t bytes_done;
    g_sdio_error = rp2040_sdio_tx_poll(&bytes_done);

    if (callback) {
      callback(m_stream_count_start + bytes_done);
    }
  } while (g_sdio_error == SDIO_BUSY);

  if (g_sdio_error != SDIO_OK) {
    Trace::Log("SDIO", "SdioCard::writeSector(", sector,
               ") failed: ", (int)g_sdio_error);
  }

  return g_sdio_error == SDIO_OK;
}

bool SdioCard::writeSectors(uint32_t sector, const uint8_t *src, size_t n) {
  if (((uint32_t)src & 3) != 0) {
    // Unaligned write, execute sector-by-sector
    for (size_t i = 0; i < n; i++) {
      if (!writeSector(sector + i, src + 512 * i)) {
        return false;
      }
    }
    return true;
  }

  sd_callback_t callback =
      get_stream_callback(src, n * 512, "writeSectors", sector);

  uint32_t reply;
  if (!checkReturnOk(rp2040_sdio_command_R1(16, 512, &reply)) || // SET_BLOCKLEN
      !checkReturnOk(
          rp2040_sdio_command_R1(CMD55, g_sdio_rca, &reply)) || // APP_CMD
      !checkReturnOk(rp2040_sdio_command_R1(
          ACMD23, n, &reply)) || // SET_WR_CLK_ERASE_COUNT
      !checkReturnOk(rp2040_sdio_command_R1(CMD25, sector,
                                            &reply)) || // WRITE_MULTIPLE_BLOCK
      !checkReturnOk(rp2040_sdio_tx_start(src, n)))     // Start transmission
  {
    return false;
  }

  do {
    uint32_t bytes_done;
    g_sdio_error = rp2040_sdio_tx_poll(&bytes_done);

    if (callback) {
      callback(m_stream_count_start + bytes_done);
    }
  } while (g_sdio_error == SDIO_BUSY);

  if (g_sdio_error != SDIO_OK) {
    Trace::Log("SDIO", "SdioCard::writeSectors(", sector, ",...,", (int)n,
               ") failed: ", (int)g_sdio_error);
    stopTransmission(true);
    return false;
  } else {
    return stopTransmission(true);
  }
}

bool SdioCard::readSector(uint32_t sector, uint8_t *dst) {
  uint8_t *real_dst = dst;
  if (((uint32_t)dst & 3) != 0) {
    // Buffer is not aligned, need to memcpy() the data from a temporary buffer.
    dst = (uint8_t *)g_sdio_dma_buf;
  }

  sd_callback_t callback = get_stream_callback(dst, 512, "readSector", sector);

  uint32_t reply;
  if (!checkReturnOk(rp2040_sdio_command_R1(16, 512, &reply)) || // SET_BLOCKLEN
      !checkReturnOk(rp2040_sdio_rx_start(dst, 1)) || // Prepare for reception
      !checkReturnOk(
          rp2040_sdio_command_R1(CMD17, sector, &reply))) // READ_SINGLE_BLOCK
  {
    return false;
  }

  do {
    uint32_t bytes_done;
    g_sdio_error = rp2040_sdio_rx_poll(&bytes_done);

    if (callback) {
      callback(m_stream_count_start + bytes_done);
    }
  } while (g_sdio_error == SDIO_BUSY);

  if (g_sdio_error != SDIO_OK) {
    Trace::Log("SDIO", "SdioCard::readSector(", sector,
               ") failed: ", (int)g_sdio_error);
  }

  if (dst != real_dst) {
    memcpy(real_dst, g_sdio_dma_buf, sizeof(g_sdio_dma_buf));
  }

  return g_sdio_error == SDIO_OK;
}

bool SdioCard::readSectors(uint32_t sector, uint8_t *dst, size_t n) {
  if (((uint32_t)dst & 3) != 0 || sector + n >= g_sdio_sector_count) {
    // Unaligned read or end-of-drive read, execute sector-by-sector
    for (size_t i = 0; i < n; i++) {
      if (!readSector(sector + i, dst + 512 * i)) {
        return false;
      }
    }
    return true;
  }

  sd_callback_t callback =
      get_stream_callback(dst, n * 512, "readSectors", sector);

  uint32_t reply;
  if (!checkReturnOk(rp2040_sdio_command_R1(16, 512, &reply)) || // SET_BLOCKLEN
      !checkReturnOk(rp2040_sdio_rx_start(dst, n)) || // Prepare for reception
      !checkReturnOk(
          rp2040_sdio_command_R1(CMD18, sector, &reply))) // READ_MULTIPLE_BLOCK
  {
    return false;
  }

  do {
    uint32_t bytes_done;
    g_sdio_error = rp2040_sdio_rx_poll(&bytes_done);

    if (callback) {
      callback(m_stream_count_start + bytes_done);
    }
  } while (g_sdio_error == SDIO_BUSY);

  if (g_sdio_error != SDIO_OK) {
    Trace::Log("SDIO", "SdioCard::readSectors(", sector, ",...,", (int)n,
               ") failed: ", (int)g_sdio_error);
    stopTransmission(true);
    return false;
  } else {
    return stopTransmission(true);
  }
}

// These functions are not used for SDIO mode but are needed to avoid build
// error.
void sdCsInit(SdCsPin_t pin) {}
void sdCsWrite(SdCsPin_t pin, bool level) {}

// SDIO configuration for main program
SdioConfig g_sd_sdio_config(DMA_SDIO);
