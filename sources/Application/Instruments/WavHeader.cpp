/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "WavHeader.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/I_File.h"

bool WavHeaderWriter::WriteHeader(I_File *file, uint32_t sampleRate,
                                  uint16_t channels, uint16_t bitsPerSample) {
  if (!file)
    return false;

  // RIFF chunk
  uint32_t chunk = 0x46464952; // "RIFF"
  if (file->Write(&chunk, 1, 4) != 4)
    return false;

  uint32_t size = 0; // Placeholder, to be filled later
  if (file->Write(&size, 1, 4) != 4)
    return false;

  // WAVE chunk
  chunk = 0x45564157; // "WAVE"
  if (file->Write(&chunk, 1, 4) != 4)
    return false;

  // "fmt " subchunk
  chunk = 0x20746D66; // "fmt "
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
  chunk = 0x61746164; // "data"
  if (file->Write(&chunk, 1, 4) != 4)
    return false;

  size = 0; // Placeholder, to be updated later
  if (file->Write(&size, 1, 4) != 4)
    return false;

  file->Sync();
  return true;
}

std::expected<WavHeaderInfo, WAVEFILE_ERROR>
WavHeaderWriter::ReadHeader(I_File *file) {
  if (!file) {
    return std::unexpected(INVALID_FILE);
  }

  file->Seek(0, SEEK_SET);

  WavHeaderInfo info;

  uint32_t chunk = 0;
  if (file->Read(&chunk, 4) != 4) {
    return std::unexpected(INVALID_HEADER);
  }

  if (chunk != 0x46464952) { // "RIFF"
    Trace::Error("WavHeaderWriter: Missing RIFF identifier");
    return std::unexpected(UNSUPPORTED_FILE_FORMAT);
  }

  if (file->Read(&info.riffChunkSize, 4) != 4) {
    return std::unexpected(INVALID_HEADER);
  }

  if (file->Read(&chunk, 4) != 4) {
    return std::unexpected(INVALID_HEADER);
  }

  if (chunk != 0x45564157) { // "WAVE"
    Trace::Error("WavHeaderWriter: Missing WAVE identifier");
    return std::unexpected(UNSUPPORTED_WAV_FORMAT);
  }

  const uint32_t riffEnd = info.riffChunkSize + 8;
  bool fmtFound = false;

  while (!fmtFound) {
    if (file->Read(&chunk, 4) != 4) {
      return std::unexpected(INVALID_HEADER);
    }

    uint32_t chunkSize = 0;
    if (file->Read(&chunkSize, 4) != 4) {
      return std::unexpected(INVALID_HEADER);
    }
    info.fmtChunkSize = chunkSize;

    uint32_t chunkDataOffset = file->Tell();
    uint32_t paddedChunkSize = info.fmtChunkSize + (info.fmtChunkSize & 1);
    uint32_t nextOffset = chunkDataOffset + paddedChunkSize;
    if (nextOffset > riffEnd) {
      Trace::Error(
          "WavHeaderWriter: fmt chunk exceeds RIFF bounds (chunkSize=%u "
          "padded=%u dataOffset=%u riffEnd=%u)",
          info.fmtChunkSize, paddedChunkSize, chunkDataOffset, riffEnd);
      return std::unexpected(INVALID_HEADER);
    }

    if (chunk == 0x20746D66) { // "fmt "
      fmtFound = true;
      break;
    }

    file->Seek(info.fmtChunkSize, SEEK_CUR);

    if (info.fmtChunkSize & 1) {
      file->Seek(1, SEEK_CUR);
    }
  }

  if (!fmtFound) {
    Trace::Error("WavHeaderWriter: fmt chunk missing");
    return std::unexpected(INVALID_HEADER);
  }

  if (info.fmtChunkSize < 16) {
    Trace::Error("WavHeaderWriter: fmt chunk too small");
    return std::unexpected(INVALID_HEADER);
  }

  if (file->Read(&info.audioFormat, 2) != 2) {
    return std::unexpected(INVALID_HEADER);
  }

  if (info.audioFormat != 1) {
    Trace::Error("WavHeaderWriter: Unsupported compression format %u",
                 info.audioFormat);
    return std::unexpected(UNSUPPORTED_COMPRESSION);
  }

  if (file->Read(&info.numChannels, 2) != 2) {
    return std::unexpected(INVALID_HEADER);
  }

  if (info.numChannels == 0) {
    Trace::Error("WavHeaderWriter: Invalid channel count 0");
    return std::unexpected(INVALID_HEADER);
  }

  if (file->Read(&info.sampleRate, 4) != 4) {
    return std::unexpected(INVALID_HEADER);
  }

  if (info.sampleRate > 44100) {
    Trace::Error("WavHeaderWriter: Unsupported sample rate %u",
                 info.sampleRate);
    return std::unexpected(UNSUPPORTED_SAMPLERATE);
  }

  if (file->Read(&info.byteRate, 4) != 4) {
    return std::unexpected(INVALID_HEADER);
  }

  if (file->Read(&info.blockAlign, 2) != 2) {
    return std::unexpected(INVALID_HEADER);
  }

  if (file->Read(&info.bitsPerSample, 2) != 2) {
    return std::unexpected(INVALID_HEADER);
  }

  if ((info.bitsPerSample != 8) && (info.bitsPerSample != 16)) {
    Trace::Error("WavHeaderWriter: Unsupported bit depth %u",
                 info.bitsPerSample);
    return std::unexpected(UNSUPPORTED_BITDEPTH);
  }

  info.bytesPerSample = info.bitsPerSample / 8;

  if (info.fmtChunkSize > 16) {
    uint32_t toSkip = info.fmtChunkSize - 16;
    if (file->Tell() + toSkip > riffEnd) {
      Trace::Error("WavHeaderWriter: fmt extra data exceeds RIFF bounds");
      return std::unexpected(INVALID_HEADER);
    }
    file->Seek(toSkip, SEEK_CUR);
  }
  if (info.fmtChunkSize & 1) {
    file->Seek(1, SEEK_CUR);
  }

  while (true) {
    if (file->Read(&chunk, 4) != 4) {
      return std::unexpected(INVALID_HEADER);
    }

    uint32_t chunkSize = 0;
    if (file->Read(&chunkSize, 4) != 4) {
      return std::unexpected(INVALID_HEADER);
    }

    uint32_t dataStart = file->Tell();
    uint32_t paddedChunkSize = chunkSize + (chunkSize & 1);
    uint32_t chunkEnd = dataStart + paddedChunkSize;
    if (chunkEnd > riffEnd) {
      Trace::Error("WavHeaderWriter: data chunk exceeds RIFF bounds "
                   "(chunk=%u chunkSize=%u chunkEnd=%u riffEnd=%u)",
                   chunk, chunkSize, chunkEnd, riffEnd);
      return std::unexpected(INVALID_HEADER);
    }

    if (chunk == 0x61746164) { // "data"
      info.dataChunkSize = chunkSize;
      info.dataOffset = dataStart;
      break;
    }

    file->Seek(chunkSize, SEEK_CUR);

    if (chunkSize & 1) {
      file->Seek(1, SEEK_CUR);
    }

    if (static_cast<uint32_t>(file->Tell()) >= riffEnd) {
      Trace::Error("WavHeaderWriter: data chunk not found within RIFF bounds");
      return std::unexpected(INVALID_HEADER);
    }
  }

  if (info.dataChunkSize == 0) {
    Trace::Error("WavHeaderWriter: Missing or empty data chunk");
    return std::unexpected(INVALID_HEADER);
  }

  file->Seek(info.dataOffset, SEEK_SET);
  return info;
}
bool WavHeaderWriter::UpdateFileSize(I_File *file, uint16_t channels,
                                     uint16_t bytesPerSample) {
  if (!file) {
    return false;
  }

  // Use the actual filesize as the sample count may contain samples there were
  // dropped and not actually written to the output file
  uint32_t totalFileSize = file->Tell();
  (void)channels;
  (void)bytesPerSample;
  if (totalFileSize < 8) {
    Trace::Error("WavHeaderWriter: File too small to contain WAV header");
    return false;
  }

  // Calculate the two size fields required by the WAV header
  uint32_t chunk_size = totalFileSize - 8;
  uint32_t subchunk2_size = (totalFileSize >= 44) ? (totalFileSize - 44) : 0;

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
