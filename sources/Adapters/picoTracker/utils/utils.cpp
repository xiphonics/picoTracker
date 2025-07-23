/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "utils.h"
#ifdef SDIO_BENCH
#include "Externals/SdFat/src/SdFat.h"
#endif
#include "System/System/System.h"
#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include <System/Console/Trace.h>
#include <stdio.h>

uint32_t measure_free_mem(void) {
  void *buff[256];
  uint32_t max = 0;

  int i = 0;
  for (; i < 256; i++) {
    buff[i] = malloc(1000);
    if (buff[i]) {
      max = i;
    } else {
      break;
    }
  }
  for (int j = i; j >= 0; j--) {
    free(buff[j]);
  }

  Trace::Debug("MAX memory free in heap: %i", max * 1000);
  /*
    buff = malloc(80000);
  if (buff) {
    Trace::Debug("MALLOC addr: %p %i - Mem free: %i", buff,
           reinterpret_cast<uintptr_t>(buff),  0x20040000l -
               reinterpret_cast<uintptr_t>(buff));
    free(buff);
    }*/
  return max;
}

void measure_freqs(void) {
  uint f_pll_sys =
      frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY);
  uint f_pll_usb =
      frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_USB_CLKSRC_PRIMARY);
  uint f_rosc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_ROSC_CLKSRC);
  uint f_clk_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
  uint f_clk_peri = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_PERI);
  uint f_clk_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_USB);
  uint f_clk_adc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_ADC);
#if PICO_RP2040
  uint f_clk_rtc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_RTC);
#endif

  Trace::Debug("pll_sys  = %dkHz", f_pll_sys);
  Trace::Debug("pll_usb  = %dkHz", f_pll_usb);
  Trace::Debug("rosc     = %dkHz", f_rosc);
  Trace::Debug("clk_sys  = %dkHz", f_clk_sys);
  Trace::Debug("clk_peri = %dkHz", f_clk_peri);
  Trace::Debug("clk_usb  = %dkHz", f_clk_usb);
  Trace::Debug("clk_adc  = %dkHz", f_clk_adc);
#if PICO_RP2040
  Trace::Debug("clk_rtc  = %dkHz", f_clk_rtc);
#endif

  // Can't measure clk_ref / xosc as it is the ref
}

#ifdef SDIO_BENCH
#define SD_CONFIG SdioConfig(FIFO_SDIO)

void cidDmp(SdFs *sd) {
  cid_t cid;
  if (!sd->card()->readCID(&cid)) {
    Trace::Debug("E: readCID failed");
  }
  Trace::Debug("Manufacturer ID: %#04x", int(cid.mid));
  Trace::Debug("OEM ID: %s%s", cid.oid[0], cid.oid[1]);
  Trace::Debug("Product: %s", cid.pnm);

  Trace::Debug("Revision: %i.%i", cid.prvN(), cid.prvM());
  Trace::Debug("Serial number: %#10x", cid.psn());
  Trace::Debug("Manufacturing date: %i/%i", cid.mdtMonth(), cid.mdtYear());
}

void sd_bench() {

  const bool PRE_ALLOCATE = true;

  // Set SKIP_FIRST_LATENCY true if the first read/write to the SD can
  // be avoid by writing a file header or reading the first record.
  const bool SKIP_FIRST_LATENCY = true;

  // Size of read/write.
  const size_t BUF_SIZE = 512;

  // File size in MB where MB = 1,000,000 bytes.
  const uint32_t FILE_SIZE_MB = 5;

  // Write pass count.
  const uint8_t WRITE_COUNT = 2;

  // Read pass count.
  const uint8_t READ_COUNT = 2;
  //==============================================================================
  // End of configuration constants.
  //------------------------------------------------------------------------------
  // File size in bytes.
  const uint32_t FILE_SIZE = 1000000UL * FILE_SIZE_MB;

  // Insure 4-byte alignment.
  uint32_t buf32[(BUF_SIZE + 3) / 4];
  uint8_t *buf = (uint8_t *)buf32;

  SdFs sd;
  FsFile file;

  float s;
  uint32_t t;
  uint32_t maxLatency;
  uint32_t minLatency;
  uint32_t totalLatency;
  bool skipLatency;

  // Discard any input.
  //  clearSerialInput();

  // F() stores strings in flash to save RAM
  //  cout << F("Type any character to start");
  //  while (!Serial.available()) {
  //    yield();
  //  }

  if (!sd.begin(SD_CONFIG)) {
    //    sd.initErrorHalt(&Serial);
  }
  if (sd.fatType() == FAT_TYPE_EXFAT) {
    Trace::Debug("Type is exFAT");
  } else {
    Trace::Debug("Type is FAT %i", int(sd.fatType()));
  }

  Trace::Debug("Card size: %i", sd.card()->sectorCount() * 512E-9);
  Trace::Debug(" GB (GB = 1E9 bytes)");

  cidDmp(&sd);

  // open or create file - truncate existing file.
  if (!file.open("bench.dat", O_RDWR | O_CREAT | O_TRUNC)) {
    Trace::Debug("E: open failed");
  }

  // fill buf with known data
  if (BUF_SIZE > 1) {
    for (size_t i = 0; i < (BUF_SIZE - 2); i++) {
      buf[i] = 'A' + (i % 26);
    }
    buf[BUF_SIZE - 2] = '\r';
  }
  buf[BUF_SIZE - 1] = '\n';

  Trace::Debug("FILE_SIZE_MB = %i", FILE_SIZE_MB);
  Trace::Debug("BUF_SIZE = %i bytes", BUF_SIZE);
  Trace::Debug("Starting write test, please wait.");

  // do write test
  uint32_t n = FILE_SIZE / BUF_SIZE;
  Trace::Debug("write speed and latency");
  Trace::Debug("speed,max,min,avg");
  Trace::Debug("KB/Sec,usec,usec,usec");
  for (uint8_t nTest = 0; nTest < WRITE_COUNT; nTest++) {
    file.truncate(0);
    if (PRE_ALLOCATE) {
      if (!file.preAllocate(FILE_SIZE)) {
        Trace::Debug("E: preAllocate failed");
      }
    }
    maxLatency = 0;
    minLatency = 9999999;
    totalLatency = 0;
    skipLatency = SKIP_FIRST_LATENCY;
    t = millis();
    for (uint32_t i = 0; i < n; i++) {
      uint32_t m = micros();
      if (file.write(buf, BUF_SIZE) != BUF_SIZE) {
        Trace::Debug("E: write failed");
      }
      m = micros() - m;
      totalLatency += m;
      if (skipLatency) {
        // Wait until first write to SD, not just a copy to the cache.
        skipLatency = file.curPosition() < 512;
      } else {
        if (maxLatency < m) {
          maxLatency = m;
        }
        if (minLatency > m) {
          minLatency = m;
        }
      }
    }
    file.sync();
    t = millis() - t;
    s = file.fileSize();
    Trace::Debug("%i,%i,%i,%i", s / t, maxLatency, minLatency,
                 totalLatency / n);
  }
  Trace::Debug("Starting read test, please wait.");
  Trace::Debug("read speed and latency");
  Trace::Debug("speed,max,min,avg");
  Trace::Debug("KB/Sec,usec,usec,usec");

  // do read test
  for (uint8_t nTest = 0; nTest < READ_COUNT; nTest++) {
    file.rewind();
    maxLatency = 0;
    minLatency = 9999999;
    totalLatency = 0;
    skipLatency = SKIP_FIRST_LATENCY;
    t = millis();
    for (uint32_t i = 0; i < n; i++) {
      buf[BUF_SIZE - 1] = 0;
      uint32_t m = micros();
      int32_t nr = file.read(buf, BUF_SIZE);
      if (nr != BUF_SIZE) {
        Trace::Debug("E: read failed");
      }
      m = micros() - m;
      totalLatency += m;
      if (buf[BUF_SIZE - 1] != '') {

        Trace::Debug("E: data check error");
      }
      if (skipLatency) {
        skipLatency = false;
      } else {
        if (maxLatency < m) {
          maxLatency = m;
        }
        if (minLatency > m) {
          minLatency = m;
        }
      }
    }
    s = file.fileSize();
    t = millis() - t;
    Trace::Debug("%i,%i,%i,%i", s / t, maxLatency, minLatency,
                 totalLatency / n);
  }
  Trace::Debug("Done");
  file.close();
  sd.end();
}
#endif
