//
// swdloader.h
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
#ifndef _pico_swdloader_h
#define _pico_swdloader_h

#include "pico/stdlib.h"
#include <cstdio>

class CSWDLoader /// Loads a program via the Serial Wire Debug interface to the
                 /// RP2040
{
public:
  const static unsigned DefaultClockRateKHz =
      400; ///< Default clock rate in KHz

public:
  /// \param nClockPin GPIO pin to which SWCLK is connected
  /// \param nDataPin GPIO pin to which SWDIO is connected
  /// \param nResetPin Optional GPIO pin to which RESET (RUN) is connected
  /// (active LOW) \param nClockRateKHz Requested interface clock rate in KHz
  /// \note GPIO pin numbers are SoC number, not header positions.
  /// \note The actual clock rate may be smaller than the requested.
  CSWDLoader();

  ~CSWDLoader(void);

  /// \brief Reset RP2040 and attach to SW debug port
  /// \return Operation successful?
  bool Initialize(void);

  /// \brief Halt the RP2040, load a program image and start it
  /// \param pProgram Pointer to program image in memory
  /// \param nProgSize Size of the program image (must be a multiple of 4)
  /// \param nAddress Load and start address of the program image
  bool Load(const void *pProgram, size_t nProgSize, uint32_t nAddress);

public:
  /// \brief Halt the RP2040
  /// \return Operation successful?
  bool Halt(void);

  /// \brief Load a chunk of a program image (or entire program)
  /// \param pChunk Pointer to the chunk in memory
  /// \param nChunkSize Size of the chunk (must be a multiple of 4)
  /// \param nAddress Load address of the chunk
  /// \return Operation successful?
  bool LoadChunk(const void *pChunk, size_t nChunkSize, uint32_t nAddress);

  /// \brief Start program image
  /// \param nAddress Start address of the program image
  /// \return Operation successful?
  bool Start(uint32_t nAddress);

private:
  bool PowerOn(void);

  bool WriteMem(uint32_t nAddress, uint32_t nData);
  bool ReadMem(uint32_t nAddress, uint32_t *pData);

  bool WriteData(uint8_t uchRequest, uint32_t nData);
  bool ReadData(uint8_t uchRequest, uint32_t *pData);

  void SelectTarget(uint32_t nCPUAPID, uint8_t uchInstanceID);

  void BeginTransaction(void);
  void EndTransaction(void);

  void Dormant2SWD(void);
  void LineReset(void);
  void WriteIdle(void);

  void WriteBits(uint32_t nBits, unsigned nBitCount);
  uint32_t ReadBits(unsigned nBitCount);
  void WriteClock(void);
};

#endif
