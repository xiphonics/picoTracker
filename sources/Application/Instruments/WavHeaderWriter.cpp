/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "WavHeaderWriter.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/I_File.h"
#include "WavFile.h"

bool WavHeaderWriter::WriteHeader(I_File *file, uint32_t sampleRate,
                                  uint16_t channels, uint16_t bitsPerSample) {
  if (!file)
    return false;

  // RIFF chunk
  uint32_t chunk = Swap32(0x46464952); // "RIFF"
  if (file->Write(&chunk, 1, 4) != 4)
    return false;

  uint32_t size = 0; // to be filled later
  if (file->Write(&size, 1, 4) != 4)
    return false;

  // WAVE chunk
  chunk = Swap32(0x45564157); // "WAVE"
  if (file->Write(&chunk, 1, 4) != 4)
    return false;

  chunk = Swap32(0x20746D66); // "fmt "
  if (file->Write(&chunk, 1, 4) != 4)
    return false;

  size = Swap32(16); // fmt chunk size
  if (file->Write(&size, 1, 4) != 4)
    return false;

  uint16_t ushort = Swap16(1); // PCM compression
  if (file->Write(&ushort, 1, 2) != 2)
    return false;

  ushort = Swap16(channels); // number of channels
  if (file->Write(&ushort, 1, 2) != 2)
    return false;

  uint32_t swappedSampleRate = Swap32(sampleRate);
  if (file->Write(&swappedSampleRate, 1, 4) != 4)
    return false;

  uint32_t byteRate = Swap32((bitsPerSample / 8) * channels * sampleRate);
  if (file->Write(&byteRate, 1, 4) != 4)
    return false;

  ushort = Swap16((bitsPerSample / 8) * channels); // block align
  if (file->Write(&ushort, 1, 2) != 2)
    return false;

  ushort = Swap16(bitsPerSample); // bits per sample
  if (file->Write(&ushort, 1, 2) != 2)
    return false;

  // data subchunk
  chunk = Swap32(0x61746164); // "data"
  if (file->Write(&chunk, 1, 4) != 4)
    return false;

  size = 0; // to be updated later
  if (file->Write(&size, 1, 4) != 4)
    return false;

  return true;
}

bool WavHeaderWriter::UpdateFileSize(I_File *file, uint32_t sampleCount) {
  if (!file)
    return false;

  // Calculate data size (sampleCount * channels * bytes per sample)
  uint32_t dataSize = sampleCount * 2 * 2; // stereo * 16-bit

  // Update total file size (file size - 8 bytes for RIFF header)
  size_t currentPos = file->Tell();
  file->Seek(4, SEEK_SET);
  uint32_t totalSize = Swap32(currentPos - 8);
  file->Write(&totalSize, 4, 1);

  // Update data chunk size
  file->Seek(40, SEEK_SET);
  uint32_t swappedDataSize = Swap32(dataSize);
  file->Write(&swappedDataSize, 4, 1);

  file->Seek(currentPos, SEEK_SET);
  return true;
}
