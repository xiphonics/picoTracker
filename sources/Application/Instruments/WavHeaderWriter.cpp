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

  uint32_t size = 0; // Placeholder, to be filled later
  if (file->Write(&size, 1, 4) != 4)
    return false;

  // WAVE chunk
  chunk = Swap32(0x45564157); // "WAVE"
  if (file->Write(&chunk, 1, 4) != 4)
    return false;

  // "fmt " subchunk
  chunk = Swap32(0x20746D66); // "fmt "
  if (file->Write(&chunk, 1, 4) != 4)
    return false;

  size = 16; // fmt chunk size for PCM
  if (file->Write(&size, 1, 4) != 4)
    return false;

  uint16_t ushort = 1; // PCM compression
  if (file->Write(&ushort, 1, 2) != 2)
    return false;

  ushort = channels; // number of channels
  if (file->Write(&ushort, 1, 2) != 2)
    return false;

  if (file->Write(&sampleRate, 1, 4) != 4)
    return false;

  uint32_t byteRate = (bitsPerSample / 8) * channels * sampleRate;
  if (file->Write(&byteRate, 1, 4) != 4)
    return false;

  ushort = (bitsPerSample / 8) * channels; // block align
  if (file->Write(&ushort, 1, 2) != 2)
    return false;

  ushort = bitsPerSample; // bits per sample
  if (file->Write(&ushort, 1, 2) != 2)
    return false;

  // data subchunk
  chunk = Swap32(0x61746164); // "data"
  if (file->Write(&chunk, 1, 4) != 4)
    return false;

  size = 0; // Placeholder, to be updated later
  if (file->Write(&size, 1, 4) != 4)
    return false;

  file->Sync();
  return true;
}

bool WavHeaderWriter::UpdateFileSize(I_File *file, uint32_t sampleCount,
                                     uint16_t channels,
                                     uint16_t bytesPerSample) {
  if (!file) {
    return false;
  }

  // Get the current position, which is the total file size
  uint32_t totalFileSize = file->Tell();

  // Calculate the two size fields required by the WAV header
  uint32_t chunk_size = totalFileSize - 8;
  uint32_t subchunk2_size = sampleCount * channels * bytesPerSample;

  // Update ChunkSize (Total file size - 8)
  file->Seek(4, SEEK_SET);
  file->Write(&chunk_size, 4, 1);

  Trace::Log("WAVHEADER", "Updating header: FileSize=%u, DataSize=%u",
             chunk_size, subchunk2_size);

  // Update Subchunk2Size (the size of the raw data)
  file->Seek(40, SEEK_SET);
  file->Write(&subchunk2_size, 4, 1);

  // Return the file pointer to its original position at the end of the file
  file->Seek(totalFileSize, SEEK_SET);

  // Force a sync to write all cached data to the disk before closing.
  file->Sync();
  return true;
}