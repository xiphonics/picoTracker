//
// swdloader.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2021  R. Stange <rsta2@o2online.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include "swdloader.h"

#define DSP_SWIO 28
#define DSP_SWCLK 29

// UTIL
#define BIT(n) (1UL << (n))

uint32_t parity32(unsigned nValue) {
  uint32_t nResult = 0;

  while (nValue != 0) {
    nResult ^= (nValue & 1);
    nValue >>= 1;
  }

  return nResult;
}

//
// References:
//
// [1] ARM Debug Interface Architecture Specification ADIv5.0 to ADIv5.2, IHI
// 0031E [2] ARM v6-M Architecture Reference Manual, DDI 0419E
//

// Debug Port v2

#define TURN_CYCLES 1

// SWD-DP Requests
#define WR_DP_ABORT 0x81
#define DP_ABORT_STKCMPCLR BIT(1)
#define DP_ABORT_STKERRCLR BIT(2)
#define DP_ABORT_WDERRCLR BIT(3)
#define DP_ABORT_ORUNERRCLR BIT(4)
#define RD_DP_CTRL_STAT 0x8D
#define WR_DP_CTRL_STAT 0xA9
#define DP_CTRL_STAT_ORUNDETECT BIT(0)
#define DP_CTRL_STAT_STICKYERR BIT(5)
#define DP_CTRL_STAT_CDBGPWRUPREQ BIT(28)
#define DP_CTRL_STAT_CDBGPWRUPACK BIT(29)
#define DP_CTRL_STAT_CSYSPWRUPREQ BIT(30)
#define DP_CTRL_STAT_CSYSPWRUPACK BIT(31)
#define RD_DP_DPIDR 0xA5
#define DP_DPIDR_SUPPORTED 0x0BC12477
#define RD_DP_RDBUFF 0xBD
#define WR_DP_SELECT 0xB1
#define DP_SELECT_DPBANKSEL__SHIFT 0
#define DP_SELECT_APBANKSEL__SHIFT 4
#define DP_SELECT_APSEL__SHIFT 24
#define DP_SELECT_DEFAULT 0 // DP bank 0, AP 0, AP bank 0
#define WR_DP_TARGETSEL 0x99
#define DP_TARGETSEL_CPUAPID_SUPPORTED 0x01002927
#define DP_TARGETSEL_TINSTANCE__SHIFT 28
#define DP_TARGETSEL_TINSTANCE_CORE0 0
#define DP_TARGETSEL_TINSTANCE_CORE1 1

// SW-DP response
#define DP_OK 0b001
#define DP_WAIT 0b010
#define DP_FAULT 0b100

// SWD MEM-AP Requests
#define WR_AP_CSW 0xA3
#define AP_CSW_SIZE__SHIFT 0
#define AP_CSW_SIZE_32BITS 2
#define AP_CSW_ADDR_INC__SHIFT 4
#define AP_CSW_SIZE_INCREMENT_SINGLE 1
#define AP_CSW_DEVICE_EN BIT(6)
#define AP_CSW_PROT__SHIFT 24
#define AP_CSW_PROT_DEFAULT 0x22
#define AP_CSW_DBG_SW_ENABLE BIT(31)
#define RD_AP_DRW 0x9F
#define WR_AP_DRW 0xBB
#define WR_AP_TAR 0x8B

// ARMv6-M Debug System Registers
#define DHCSR 0xE000EDF0
#define DHCSR_C_DEBUGEN BIT(0)
#define DHCSR_C_HALT BIT(1)
#define DHCSR_DBGKEY__SHIFT 16
#define DHCSR_DBGKEY_KEY 0xA05F
#define DCRSR 0xE000EDF4
#define DCRSR_REGSEL__SHIFT 0
#define DCRSR_REGSEL_R15 15 // PC register
#define DCRSR_REGW_N_R BIT(16)
#define DCRDR 0xE000EDF8

CSWDLoader::CSWDLoader() {
  gpio_init(DSP_SWIO);
  gpio_set_dir(DSP_SWIO, true);
  gpio_init(DSP_SWCLK);
  gpio_set_dir(DSP_SWCLK, true);

  gpio_pull_up(DSP_SWIO);
}

CSWDLoader::~CSWDLoader(void) {
  gpio_set_dir(DSP_SWIO, false);
  gpio_set_dir(DSP_SWCLK, false);
}

bool CSWDLoader::Initialize(void) {
  BeginTransaction();

  Dormant2SWD();
  WriteIdle();
  LineReset();

  // core 1 remains halted after reset
  SelectTarget(DP_TARGETSEL_CPUAPID_SUPPORTED, DP_TARGETSEL_TINSTANCE_CORE0);

  uint32_t nIDCode;
  if (!ReadData(RD_DP_DPIDR, &nIDCode)) {
    printf("W: Target does not respond\n");

    return false;
  }

  if (nIDCode != DP_DPIDR_SUPPORTED) {
    EndTransaction();

    printf("E: Debug target not supported (ID code 0x%X)\n", nIDCode);

    return false;
  }

  if (!PowerOn()) {
    printf("E: Target connect failed\n");

    return false;
  }

  EndTransaction();

  return true;
}

bool CSWDLoader::Load(const void *pProgram, size_t nProgSize,
                      uint32_t nAddress) {
  if (!Halt()) {
    return false;
  }

  unsigned nStartTime = to_us_since_boot(get_absolute_time());

  if (!LoadChunk(pProgram, nProgSize, nAddress)) {
    return false;
  }

  unsigned nEndTime = to_us_since_boot(get_absolute_time());
  double fDuration = (double)(nEndTime - nStartTime) / 1000000;

  printf("N: %u bytes loaded in %.2f seconds (%.1f KBytes/s)\n", nProgSize,
         fDuration, nProgSize / fDuration / 1024.0);

  return Start(nAddress);
}

bool CSWDLoader::Halt(void) {
  BeginTransaction();

  if (!WriteData(WR_AP_CSW,
                 (AP_CSW_SIZE_32BITS << AP_CSW_SIZE__SHIFT) |
                     (AP_CSW_SIZE_INCREMENT_SINGLE << AP_CSW_ADDR_INC__SHIFT) |
                     AP_CSW_DEVICE_EN |
                     (AP_CSW_PROT_DEFAULT << AP_CSW_PROT__SHIFT) |
                     AP_CSW_DBG_SW_ENABLE) ||
      !WriteMem(DHCSR, DHCSR_C_DEBUGEN | DHCSR_C_HALT |
                           (DHCSR_DBGKEY_KEY << DHCSR_DBGKEY__SHIFT))) {
    printf("E: Target halt failed\n");

    return false;
  }

  EndTransaction();

  return true;
}

bool CSWDLoader::LoadChunk(const void *pChunk, size_t nChunkSize,
                           uint32_t nAddress) {
  const uint32_t *pChunk32 = (const uint32_t *)pChunk;
  assert(pChunk32 != 0);

  uint32_t nFirstWord = *pChunk32;
  uint32_t nAddressCopy = nAddress;

  assert((nChunkSize & 3) == 0);
  while (nChunkSize > 0) {
    BeginTransaction();

    if (!WriteData(WR_AP_TAR, nAddress)) {
      printf("E: Cannot write TAR (0x%X)\n", nAddress);

      return false;
    }

    const size_t BlockSize = 1024;
    for (unsigned i = 0; i < BlockSize; i += 4) {
      if (!WriteData(WR_AP_DRW, *pChunk32++)) {
        printf("E: Memory write failed (0x%X)\n", nAddress);

        return false;
      }

      if (nChunkSize > 4) {
        nChunkSize -= 4;
      } else {
        nChunkSize = 0;
        break;
      }
    }

    nAddress += BlockSize;

    EndTransaction();
  }

  BeginTransaction();

  uint32_t nFirstWordRead;
  if (!ReadMem(nAddressCopy, &nFirstWordRead)) {
    printf("E: Memory read failed (0x%X)\n", nAddressCopy);
  }

  EndTransaction();

  if (nFirstWord != nFirstWordRead) {
    printf("E: Data mismatch (0x%X != 0x%X)\n", nFirstWord, nFirstWordRead);

    return false;
  }

  return true;
}

bool CSWDLoader::Start(uint32_t nAddress) {
  BeginTransaction();

  if (!WriteMem(DCRDR, nAddress) ||
      !WriteMem(DCRSR,
                (DCRSR_REGSEL_R15 << DCRSR_REGSEL__SHIFT) | DCRSR_REGW_N_R) ||
      !WriteMem(DHCSR,
                DHCSR_C_DEBUGEN | (DHCSR_DBGKEY_KEY << DHCSR_DBGKEY__SHIFT))) {
    printf("E: Target start failed\n");

    return false;
  }

  EndTransaction();

  return true;
}

bool CSWDLoader::PowerOn(void) {
  if (!WriteData(WR_DP_ABORT, DP_ABORT_STKCMPCLR | DP_ABORT_STKERRCLR |
                                  DP_ABORT_WDERRCLR | DP_ABORT_ORUNERRCLR)) {
    return false;
  }

  if (!WriteData(WR_DP_SELECT, DP_SELECT_DEFAULT)) {
    return false;
  }

  if (!WriteData(WR_DP_CTRL_STAT,
                 DP_CTRL_STAT_ORUNDETECT | DP_CTRL_STAT_STICKYERR |
                     DP_CTRL_STAT_CDBGPWRUPREQ | DP_CTRL_STAT_CSYSPWRUPREQ)) {
    return false;
  }

  uint32_t nCtrlStat;
  if (!ReadData(RD_DP_CTRL_STAT, &nCtrlStat)) {
    return false;
  }

  if (!(nCtrlStat & DP_CTRL_STAT_CDBGPWRUPACK) ||
      !(nCtrlStat & DP_CTRL_STAT_CSYSPWRUPACK)) {
    EndTransaction();

    return false;
  }

  return true;
}

bool CSWDLoader::WriteMem(uint32_t nAddress, uint32_t nData) {
  return WriteData(WR_AP_TAR, nAddress) && WriteData(WR_AP_DRW, nData);
}

bool CSWDLoader::ReadMem(uint32_t nAddress, uint32_t *pData) {
  return WriteData(WR_AP_TAR, nAddress) && ReadData(RD_AP_DRW, pData) &&
         ReadData(RD_DP_RDBUFF, pData);
}

bool CSWDLoader::WriteData(uint8_t nRequest, uint32_t nData) {
  WriteBits(nRequest, 7);

  assert(nRequest & 0x80);
  ReadBits(1 + TURN_CYCLES); // park bit (not driven) and turn cycle

  uint32_t nResponse = ReadBits(3);

  ReadBits(TURN_CYCLES);

  if (nResponse != DP_OK) {
    EndTransaction();

    printf("W: Cannot write (req 0x%02X, data 0x%X, resp %u)\n",
           (unsigned)nRequest, nData, nResponse);

    return false;
  }

  WriteBits(nData, 32);
  WriteBits(parity32(nData), 1);

  return true;
}

bool CSWDLoader::ReadData(uint8_t nRequest, uint32_t *pData) {
  WriteBits(nRequest, 7);

  assert(nRequest & 0x80);
  ReadBits(1 + TURN_CYCLES); // park bit (not driven) and turn cycle

  uint32_t nResponse = ReadBits(3);

  if (nResponse != DP_OK) {
    ReadBits(TURN_CYCLES);

    EndTransaction();

    printf("W: Cannot read (req 0x%02X, resp %u)\n", (unsigned)nRequest,
           nResponse);

    return false;
  }

  uint32_t nData = ReadBits(32);

  uint32_t nParity = ReadBits(1);
  if (nParity != parity32(nData)) {
    ReadBits(TURN_CYCLES);

    EndTransaction();

    printf("W: Parity error (req 0x%02X)\n", (unsigned)nRequest);

    return false;
  }

  assert(pData != 0);
  *pData = nData;

  ReadBits(TURN_CYCLES);

  return true;
}

void CSWDLoader::SelectTarget(uint32_t nCPUAPID, uint8_t uchInstanceID) {
  uint32_t nWData =
      nCPUAPID | ((uint32_t)uchInstanceID << DP_TARGETSEL_TINSTANCE__SHIFT);

  WriteBits(WR_DP_TARGETSEL, 7);

  ReadBits(1 + 5); // park bit and 5 bits not driven

  WriteBits(nWData, 32);
  WriteBits(parity32(nWData), 1);
}

void CSWDLoader::BeginTransaction(void) { WriteIdle(); }

void CSWDLoader::EndTransaction(void) { WriteIdle(); }

// Leaving dormant state and switch to SW-DP ([1] section B5.3.4)
void CSWDLoader::Dormant2SWD(void) {
  WriteBits(0xFF, 8); // 8 cycles high

  WriteBits(0x6209F392U, 32); // selection alert sequence
  WriteBits(0x86852D95U, 32);
  WriteBits(0xE3DDAFE9U, 32);
  WriteBits(0x19BC0EA2U, 32);

  WriteBits(0x0, 4); // 4 cycles low

  WriteBits(0x1A, 8); // activation code
}

void CSWDLoader::LineReset(void) {
  WriteBits(0xFFFFFFFFU, 32);
  WriteBits(0x00FFFFFU, 28);
}

void CSWDLoader::WriteIdle(void) {
  WriteBits(0, 8);

  gpio_put(DSP_SWCLK, false);

  gpio_set_dir(DSP_SWIO, true);
  gpio_put(DSP_SWIO, false);
}

void CSWDLoader::WriteBits(uint32_t nBits, unsigned nBitCount) {
  gpio_set_dir(DSP_SWIO, true);

  while (nBitCount--) {
    gpio_put(DSP_SWIO, (bool)(nBits & 1));

    WriteClock();

    nBits >>= 1;
  }
}

uint32_t CSWDLoader::ReadBits(unsigned nBitCount) {
  gpio_set_dir(DSP_SWIO, false);

  uint32_t nBits = 0;
  unsigned nRemaining = nBitCount--;
  while (nRemaining--) {
    unsigned nLevel = gpio_get(DSP_SWIO);

    WriteClock();

    nBits >>= 1;
    nBits |= nLevel << nBitCount;
  }

  return nBits;
}

void CSWDLoader::WriteClock(void) {
  gpio_put(DSP_SWCLK, false);

  // At ~400kHz and 220.5MHz clock, we need sleep for 1.25us, close enough?
  sleep_us(1);
  gpio_put(DSP_SWCLK, true);
  sleep_us(1);
}
