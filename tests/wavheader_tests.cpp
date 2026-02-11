#include "doctest/doctest.h"

#include "Application/Instruments/WavHeader.h"
#include "Application/Model/Config.h"
#include "System/FileSystem/I_File.h"

#include <cstdint>
#include <cstring>

namespace {

struct ByteWriter {
  uint8_t data[256];
  size_t size = 0;

  bool AppendBytes(const void *src, size_t len) {
    if (size + len > sizeof(data)) {
      return false;
    }
    std::memcpy(data + size, src, len);
    size += len;
    return true;
  }

  bool AppendU32(uint32_t value) {
    uint8_t bytes[4];
    bytes[0] = static_cast<uint8_t>(value & 0xFF);
    bytes[1] = static_cast<uint8_t>((value >> 8) & 0xFF);
    bytes[2] = static_cast<uint8_t>((value >> 16) & 0xFF);
    bytes[3] = static_cast<uint8_t>((value >> 24) & 0xFF);
    return AppendBytes(bytes, sizeof(bytes));
  }

  bool AppendU16(uint16_t value) {
    uint8_t bytes[2];
    bytes[0] = static_cast<uint8_t>(value & 0xFF);
    bytes[1] = static_cast<uint8_t>((value >> 8) & 0xFF);
    return AppendBytes(bytes, sizeof(bytes));
  }

  bool AppendFourCC(const char *fourcc) {
    return AppendBytes(fourcc, 4);
  }
};

class TestFile final : public I_File {
public:
  TestFile(const uint8_t *data, size_t size) : data_(data), size_(size) {}

  int Read(void *ptr, int size) override {
    if (size <= 0) {
      return 0;
    }
    size_t remaining = size_ - pos_;
    size_t to_read = static_cast<size_t>(size);
    if (to_read > remaining) {
      to_read = remaining;
    }
    if (to_read > 0) {
      std::memcpy(ptr, data_ + pos_, to_read);
      pos_ += to_read;
    }
    if (to_read < static_cast<size_t>(size)) {
      error_ = true;
    }
    return static_cast<int>(to_read);
  }

  int GetC() override {
    if (pos_ >= size_) {
      error_ = true;
      return -1;
    }
    return data_[pos_++];
  }

  int Write(const void *ptr, int size, int nmemb) override {
    (void)ptr;
    (void)size;
    (void)nmemb;
    error_ = true;
    return 0;
  }

  void Seek(long offset, int whence) override {
    size_t base = 0;
    if (whence == SEEK_CUR) {
      base = pos_;
    } else if (whence == SEEK_END) {
      base = size_;
    }

    long next = static_cast<long>(base) + offset;
    if (next < 0) {
      pos_ = 0;
      error_ = true;
    } else if (static_cast<size_t>(next) > size_) {
      pos_ = size_;
      error_ = true;
    } else {
      pos_ = static_cast<size_t>(next);
    }
  }

  long Tell() override { return static_cast<long>(pos_); }

  int Error() override { return error_ ? 1 : 0; }

  bool Sync() override { return true; }

  void Dispose() override {}

protected:
  bool Close() override { return true; }

private:
  const uint8_t *data_ = nullptr;
  size_t size_ = 0;
  size_t pos_ = 0;
  bool error_ = false;
};

ByteWriter BuildPcmWav(uint16_t channels, uint32_t sampleRate,
                      uint16_t bitsPerSample, uint32_t dataSize) {
  ByteWriter writer;
  uint32_t byteRate = sampleRate * channels * (bitsPerSample / 8);
  uint16_t blockAlign = channels * (bitsPerSample / 8);

  writer.AppendFourCC("RIFF");
  writer.AppendU32(0); // placeholder, patched later
  writer.AppendFourCC("WAVE");

  writer.AppendFourCC("fmt ");
  writer.AppendU32(16);
  writer.AppendU16(1); // PCM
  writer.AppendU16(channels);
  writer.AppendU32(sampleRate);
  writer.AppendU32(byteRate);
  writer.AppendU16(blockAlign);
  writer.AppendU16(bitsPerSample);

  writer.AppendFourCC("data");
  writer.AppendU32(dataSize);

  if (dataSize > 0) {
    uint8_t zeros[8] = {0};
    uint32_t remaining = dataSize;
    while (remaining > 0) {
      uint32_t chunk = remaining > sizeof(zeros) ? sizeof(zeros) : remaining;
      writer.AppendBytes(zeros, chunk);
      remaining -= chunk;
    }
  }

  uint32_t riffSize = static_cast<uint32_t>(writer.size - 8);
  std::memcpy(writer.data + 4, &riffSize, sizeof(riffSize));

  return writer;
}

} // namespace

TEST_CASE("ReadHeader parses valid PCM WAV") {
  Config::SetImportResampler(0);
  ByteWriter wav = BuildPcmWav(2, 44100, 16, 4);
  TestFile file(wav.data, wav.size);

  auto result = WavHeaderWriter::ReadHeader(&file);
  REQUIRE(result.has_value());

  CHECK(result->audioFormat == 1);
  CHECK(result->numChannels == 2);
  CHECK(result->sampleRate == 44100);
  CHECK(result->bitsPerSample == 16);
  CHECK(result->dataChunkSize == 4);
  CHECK(result->dataOffset > 0);
}

TEST_CASE("ReadHeader rejects missing RIFF") {
  ByteWriter wav = BuildPcmWav(2, 44100, 16, 4);
  wav.data[0] = 'N';
  TestFile file(wav.data, wav.size);

  auto result = WavHeaderWriter::ReadHeader(&file);
  REQUIRE_FALSE(result.has_value());
  CHECK(result.error() == UNSUPPORTED_FILE_FORMAT);
}

TEST_CASE("ReadHeader rejects fmt chunk too small") {
  ByteWriter wav = BuildPcmWav(2, 44100, 16, 4);
  uint32_t fmtSize = 12;
  std::memcpy(wav.data + 16, &fmtSize, sizeof(fmtSize));
  TestFile file(wav.data, wav.size);

  auto result = WavHeaderWriter::ReadHeader(&file);
  REQUIRE_FALSE(result.has_value());
  CHECK(result.error() == INVALID_HEADER);
}

TEST_CASE("ReadHeader rejects unsupported sample rate without resampling") {
  Config::SetImportResampler(0);
  ByteWriter wav = BuildPcmWav(2, 48000, 16, 4);
  TestFile file(wav.data, wav.size);

  auto result = WavHeaderWriter::ReadHeader(&file);
  REQUIRE_FALSE(result.has_value());
  CHECK(result.error() == UNSUPPORTED_SAMPLERATE);
}

TEST_CASE("ReadHeader rejects unsupported audio format") {
  ByteWriter wav = BuildPcmWav(2, 44100, 16, 4);
  uint16_t format = 2;
  std::memcpy(wav.data + 20, &format, sizeof(format));
  TestFile file(wav.data, wav.size);

  auto result = WavHeaderWriter::ReadHeader(&file);
  REQUIRE_FALSE(result.has_value());
  CHECK(result.error() == UNSUPPORTED_AUDIO_FORMAT);
}

TEST_CASE("ReadHeader accepts data chunk beyond RIFF when still within EOF") {
  Config::SetImportResampler(0);
  ByteWriter wav = BuildPcmWav(2, 44100, 16, 8);

  // Make RIFF chunk size too small by 4 bytes so data end exceeds RIFF bounds.
  uint32_t riffSize = 0;
  std::memcpy(&riffSize, wav.data + 4, sizeof(riffSize));
  riffSize -= 4;
  std::memcpy(wav.data + 4, &riffSize, sizeof(riffSize));

  TestFile file(wav.data, wav.size);
  auto result = WavHeaderWriter::ReadHeader(&file);
  REQUIRE(result.has_value());
  CHECK(result->dataChunkSize == 8);
}

TEST_CASE("ReadHeader rejects data chunk beyond EOF") {
  Config::SetImportResampler(0);
  ByteWriter wav = BuildPcmWav(2, 44100, 16, 8);

  // Increase data size field beyond available bytes in file.
  uint32_t oversizedData = 12;
  std::memcpy(wav.data + 40, &oversizedData, sizeof(oversizedData));

  TestFile file(wav.data, wav.size);
  auto result = WavHeaderWriter::ReadHeader(&file);
  REQUIRE_FALSE(result.has_value());
  CHECK(result.error() == INVALID_HEADER);
}
