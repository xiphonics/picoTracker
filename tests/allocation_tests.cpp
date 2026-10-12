#include "doctest/doctest.h"

#include "Application/Model/Chain.h"
#include "Application/Model/Phrase.h"

template <typename Model>
static void checkAllocation(uint16_t count, uint16_t exhausted) {
  Model model;
  CHECK(model.GetNext() == 0);
  model.SetUsed(0x10);
  model.SetUsed(0x11);
  CHECK(model.GetNext(0x11) == 0x12);
  CHECK(model.IsUsed(0x12));
  CHECK_FALSE(model.IsUsed(1));

  model.ClearAllocation();
  for (uint16_t i = 0x10; i < 0x20; ++i) {
    model.SetUsed(static_cast<uint8_t>(i));
  }
  CHECK(model.GetNext(0x11) == 0x20);

  model.ClearAllocation();
  model.SetUsed(static_cast<uint8_t>(count - 1));
  CHECK(model.GetNext(count) == 0);
  CHECK(model.GetNext(count) == 1);

  model.ClearAllocation();
  for (uint16_t i = 0x10; i < count; ++i) {
    model.SetUsed(static_cast<uint8_t>(i));
  }
  CHECK(model.GetNext(0x11) == 0);
  CHECK(model.GetNext(count) == 1);
  CHECK(model.GetNext(0xFF) < count);

  model.ClearAllocation();
  for (uint16_t i = 0; i < count; ++i) {
    model.SetUsed(static_cast<uint8_t>(i));
  }
  CHECK(model.GetNext(0x11) == exhausted);
  CHECK(model.GetNext(count) == exhausted);
  CHECK(model.GetNext() == exhausted);
}

TEST_CASE("Chain allocation prefers higher numbers and wraps") {
  checkAllocation<Chain>(CHAIN_COUNT, NO_MORE_CHAIN);
}

TEST_CASE("Phrase allocation prefers higher numbers and wraps") {
  checkAllocation<Phrase>(PHRASE_COUNT, NO_MORE_PHRASE);
}
