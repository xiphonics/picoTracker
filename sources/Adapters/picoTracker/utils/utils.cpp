#include "utils.h"
#include "Externals/SdFat/src/SdFat.h"
#include "System/System/System.h"
#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include <stdio.h>

uint32_t millis(void) { return to_ms_since_boot(get_absolute_time()); }

uint32_t micros(void) { return to_us_since_boot(get_absolute_time()); }

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


  printf("MAX memory free in heap: %i\n", max * 1000);
  /*
    buff = malloc(80000);
  if (buff) {
    printf("MALLOC addr: %p %i - Mem free: %i\n", buff,
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
  uint f_clk_rtc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_RTC);

  printf("pll_sys  = %dkHz\n", f_pll_sys);
  printf("pll_usb  = %dkHz\n", f_pll_usb);
  printf("rosc     = %dkHz\n", f_rosc);
  printf("clk_sys  = %dkHz\n", f_clk_sys);
  printf("clk_peri = %dkHz\n", f_clk_peri);
  printf("clk_usb  = %dkHz\n", f_clk_usb);
  printf("clk_adc  = %dkHz\n", f_clk_adc);
  printf("clk_rtc  = %dkHz\n", f_clk_rtc);

  // Can't measure clk_ref / xosc as it is the ref
}

#define SD_CONFIG SdioConfig(FIFO_SDIO)

void cidDmp(SdFs *sd) {
  cid_t cid;
  if (!sd->card()->readCID(&cid)) {
    printf("E: readCID failed\n");
  }
  printf("\nManufacturer ID: %#04x\n", int(cid.mid));
  printf("OEM ID: %s%s\n",cid.oid[0],cid.oid[1]);
  printf("Product: %s\n", cid.pnm);
  
  printf("\nRevision: %i.%i\n", cid.prvN(), cid.prvM());
  printf("Serial number: %#10x\n",cid.psn());
  printf("Manufacturing date: %i/%i\n\n",cid.mdtMonth(), cid.mdtYear());
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
  //  cout << F("Type any character to start\n");
  //  while (!Serial.available()) {
  //    yield();
  //  }

  if (!sd.begin(SD_CONFIG)) {
    //    sd.initErrorHalt(&Serial);
  }
  if (sd.fatType() == FAT_TYPE_EXFAT) {
    printf("Type is exFAT\n");
  } else {
    printf("Type is FAT %i\n", int(sd.fatType()));
  }

  printf("Card size: %i\n", sd.card()->sectorCount()*512E-9);
  printf(" GB (GB = 1E9 bytes)\n");

  cidDmp(&sd);

  // open or create file - truncate existing file.
  if (!file.open("bench.dat", O_RDWR | O_CREAT | O_TRUNC)) {
    printf("E: open failed\n");
  }

  // fill buf with known data
  if (BUF_SIZE > 1) {
    for (size_t i = 0; i < (BUF_SIZE - 2); i++) {
      buf[i] = 'A' + (i % 26);
    }
    buf[BUF_SIZE-2] = '\r';
  }
  buf[BUF_SIZE-1] = '\n';

  printf("FILE_SIZE_MB = %i\n", FILE_SIZE_MB);
  printf("BUF_SIZE = %i bytes\n", BUF_SIZE);
  printf("\nStarting write test, please wait.\n");

  // do write test
  uint32_t n = FILE_SIZE/BUF_SIZE;
  printf("write speed and latency\n");
  printf("speed,max,min,avg\n");
  printf("KB/Sec,usec,usec,usec\n");
  for (uint8_t nTest = 0; nTest < WRITE_COUNT; nTest++) {
    file.truncate(0);
    if (PRE_ALLOCATE) {
      if (!file.preAllocate(FILE_SIZE)) {
        printf("E: preAllocate failed\n");
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
        printf("E: write failed\n");
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
    printf("%i,%i,%i,%i\n", s/t, maxLatency, minLatency, totalLatency/n);
  }
  printf("\nStarting read test, please wait.\n");
  printf("\nread speed and latency\n");
  printf("speed,max,min,avg\n");
  printf("KB/Sec,usec,usec,usec\n");

  // do read test
  for (uint8_t nTest = 0; nTest < READ_COUNT; nTest++) {
    file.rewind();
    maxLatency = 0;
    minLatency = 9999999;
    totalLatency = 0;
    skipLatency = SKIP_FIRST_LATENCY;
    t = millis();
    for (uint32_t i = 0; i < n; i++) {
      buf[BUF_SIZE-1] = 0;
      uint32_t m = micros();
      int32_t nr = file.read(buf, BUF_SIZE);
      if (nr != BUF_SIZE) {
        printf("E: read failed\n");
      }
      m = micros() - m;
      totalLatency += m;
      if (buf[BUF_SIZE-1] != '\n') {

        printf("E: data check error\n");
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
    printf("%i,%i,%i,%i\n", s/t, maxLatency, minLatency, totalLatency/n);
  }
  printf("\nDone\n");
  file.close();
  sd.end();

}
