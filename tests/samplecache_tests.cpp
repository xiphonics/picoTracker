// Host tests for the pure decisions behind the project sample cache
// (issue #120). The filesystem and flash allocator sides are exercised on
// device; what is covered here is the arithmetic and the pairing rules, which
// decide whether a boot plays cached audio from flash or reloads from the SD
// card.

#include "doctest/doctest.h"

#include "Application/Instruments/SampleCacheValidate.h"

#include <cstddef>
#include <cstring>
#include <utility>

namespace {

// Build a cache entry the way the pool would.
SampleCacheEntry entry(const char *name, uint32_t flashOffset, uint32_t size,
                       uint32_t diskSize) {
  SampleCacheEntry e{};
  strncpy(e.name, name, sizeof(e.name) - 1);
  e.flashOffset = flashOffset;
  e.sampleBufferSize = size;
  e.size = size / 4;
  e.sampleRate = 44100;
  e.channelCount = 1;
  e.bytePerSample = 2;
  e.audioFormat = 1;
  e.sourceDiskSize = diskSize;
  return e;
}

// The three numbers SamplePool::validateCacheAgainstSd() derives from a card
// listing, fed through the same two helpers.
bool cardLooksLikeCache(const SampleCacheEntry *entries, size_t count,
                        const std::pair<const char *, uint32_t> *card,
                        size_t cardCount) {
  size_t unchanged = 0;
  for (size_t i = 0; i < cardCount; ++i) {
    auto pairing =
        pairCardSampleWithCache(card[i].first, card[i].second, entries, count);
    if (pairing == SampleCachePairing::Matched) {
      unchanged++;
    }
  }
  return cacheAndCardAgree(cardCount, unchanged, count);
}

constexpr uint32_t kBase = 0x10000;
constexpr uint32_t kLimit = 0x200000;

} // namespace

TEST_CASE("flashRangeFits accepts a range inside the window") {
  CHECK(flashRangeFits(kBase, 0x1000, kBase, kLimit));
  CHECK(flashRangeFits(kBase + 0x1000, 0x1000, kBase, kLimit));
}

TEST_CASE("flashRangeFits accepts a range ending exactly on the limit") {
  CHECK(flashRangeFits(kBase, kLimit - kBase, kBase, kLimit));
}

TEST_CASE("flashRangeFits rejects an offset below the window") {
  CHECK_FALSE(flashRangeFits(kBase - 0x1000, 0x1000, kBase, kLimit));
  CHECK_FALSE(flashRangeFits(0, 0x1000, kBase, kLimit));
}

TEST_CASE("flashRangeFits rejects an offset at or past the end of the window") {
  // An entry must occupy at least one byte, so it cannot start on the limit.
  CHECK_FALSE(flashRangeFits(kLimit, 0x1000, kBase, kLimit));
  CHECK_FALSE(flashRangeFits(kLimit + 0x1000, 0x1000, kBase, kLimit));
}

TEST_CASE("flashRangeFits rejects a range running past the window") {
  CHECK_FALSE(flashRangeFits(kLimit - 0x100, 0x1000, kBase, kLimit));
}

TEST_CASE("flashRangeFits is not fooled by a size that wraps") {
  // The original bug class: offset + size overflowing uint32 and comparing
  // smaller than the limit.
  CHECK_FALSE(flashRangeFits(kLimit - 0x100, 0xFFFFFFFF, kBase, kLimit));
  CHECK_FALSE(flashRangeFits(kBase, 0xFFFFFFFF, kBase, kLimit));
  CHECK_FALSE(flashRangeFits(kBase, kLimit, kBase, kLimit));
}

TEST_CASE("pairCardSampleWithCache pairs on name and confirms the size") {
  SampleCacheEntry entries[] = {
      entry("kick.wav", kBase, 0x1000, 4096),
      entry("snare.wav", kBase + 0x1000, 0x2000, 8192),
  };

  CHECK(pairCardSampleWithCache("kick.wav", 4096, entries, 2) ==
        SampleCachePairing::Matched);
  CHECK(pairCardSampleWithCache("snare.wav", 8192, entries, 2) ==
        SampleCachePairing::Matched);
}

TEST_CASE("pairCardSampleWithCache reports a file replaced on the card") {
  SampleCacheEntry entries[] = {entry("kick.wav", kBase, 0x1000, 4096)};

  // Same name, different bytes: the cached flash is not this file. This is the
  // "copy a new kick.wav over from the PC" case from the review.
  CHECK(pairCardSampleWithCache("kick.wav", 9999, entries, 1) ==
        SampleCachePairing::SizeDiffers);
}

TEST_CASE("pairCardSampleWithCache reports an added sample") {
  SampleCacheEntry entries[] = {entry("kick.wav", kBase, 0x1000, 4096)};

  CHECK(pairCardSampleWithCache("hat.wav", 2048, entries, 1) ==
        SampleCachePairing::NoMatch);
}

TEST_CASE("pairCardSampleWithCache compares names exactly") {
  // A cache hit has to reproduce the pool a fresh load builds, including the
  // sort order Load() applies, so a rename is not a match.
  SampleCacheEntry entries[] = {entry("kick.wav", kBase, 0x1000, 4096)};

  CHECK(pairCardSampleWithCache("Kick.wav", 4096, entries, 1) ==
        SampleCachePairing::NoMatch);
}

TEST_CASE("pairCardSampleWithCache with an empty cache") {
  CHECK(pairCardSampleWithCache("kick.wav", 4096, nullptr, 0) ==
        SampleCachePairing::NoMatch);
}

TEST_CASE("cacheAndCardAgree accepts an unchanged pool") {
  CHECK(cacheAndCardAgree(3, 3, 3));
  CHECK(cacheAndCardAgree(64, 64, 64));
}

TEST_CASE("cacheAndCardAgree accepts a project with no samples") {
  CHECK(cacheAndCardAgree(0, 0, 0));
}

TEST_CASE("cacheAndCardAgree rejects a sample added on the card") {
  // Would reindex the pool relative to the cached one.
  CHECK_FALSE(cacheAndCardAgree(4, 4, 3));
}

TEST_CASE("cacheAndCardAgree rejects a sample removed from the card") {
  CHECK_FALSE(cacheAndCardAgree(2, 2, 3));
}

TEST_CASE("cacheAndCardAgree rejects a replaced sample") {
  CHECK_FALSE(cacheAndCardAgree(3, 2, 3));
}

TEST_CASE("cacheAndCardAgree rejects a card listing that cannot be paired") {
  CHECK_FALSE(cacheAndCardAgree(0, 0, 3));
  CHECK_FALSE(cacheAndCardAgree(3, 0, 0));
}

TEST_CASE("end to end: card states decide whether the cache is trusted") {
  const SampleCacheEntry cached[] = {
      entry("kick.wav", kBase, 0x1000, 4096),
      entry("snare.wav", kBase + 0x1000, 0x2000, 8192),
      entry("hat.wav", kBase + 0x3000, 0x800, 2048),
  };
  constexpr size_t count = sizeof(cached) / sizeof(cached[0]);

  const std::pair<const char *, uint32_t> unchanged[] = {
      {"kick.wav", 4096}, {"snare.wav", 8192}, {"hat.wav", 2048}};
  CHECK(cardLooksLikeCache(cached, count, unchanged, 3));

  // Sample replaced from the PC without changing its size in the pool.
  const std::pair<const char *, uint32_t> replaced[] = {
      {"kick.wav", 4097}, {"snare.wav", 8192}, {"hat.wav", 2048}};
  CHECK_FALSE(cardLooksLikeCache(cached, count, replaced, 3));

  // Sample deleted through the browser but still in the cache.
  const std::pair<const char *, uint32_t> removed[] = {{"kick.wav", 4096},
                                                       {"hat.wav", 2048}};
  CHECK_FALSE(cardLooksLikeCache(cached, count, removed, 2));

  // Sample imported after the cache was written.
  const std::pair<const char *, uint32_t> added[] = {{"kick.wav", 4096},
                                                     {"snare.wav", 8192},
                                                     {"hat.wav", 2048},
                                                     {"ride.wav", 1024}};
  CHECK_FALSE(cardLooksLikeCache(cached, count, added, 4));

  // Sample renamed: same count and same sizes, different names.
  const std::pair<const char *, uint32_t> renamed[] = {
      {"kick.wav", 4096}, {"SNARE.wav", 8192}, {"hat.wav", 2048}};
  CHECK_FALSE(cardLooksLikeCache(cached, count, renamed, 3));
}
