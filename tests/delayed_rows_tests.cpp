#include "Application/Player/DelayedRows.h"
#include "doctest/doctest.h"
#include <vector>

namespace {
DelayedRow Row(uint8_t note, uint16_t delay) {
  return {.note = note,
          .instrument = 3,
          .transpose = 12,
          .cmd1 = TrackerCommand::InstrumentCommandDelay,
          .cmd2 = TrackerCommand::InstrumentCommandVolume,
          .param1 = delay,
          .param2 = 0x4080};
}
} // namespace

TEST_CASE("Every DLY value survives groove row advancement") {
  for (uint8_t groove : {1, 2, 6}) {
    for (uint16_t delay = 0; delay < 16; ++delay) {
      DelayedRows rows;
      std::vector<DelayedRow> played;
      for (uint8_t tick = 0; tick < 32; ++tick) {
        if (tick % groove == 0) {
          rows.Add(Row(tick, tick == 0 ? delay : 0));
        }
        rows.Tick([&](const DelayedRow &row) {
          if (row.note == 0) {
            CHECK(tick == delay);
            played.push_back(row);
          }
        });
      }
      REQUIRE(played.size() == 1);
      CHECK(played[0].instrument == 3);
      CHECK(played[0].transpose == 12);
      CHECK(played[0].cmd2 == TrackerCommand::InstrumentCommandVolume);
      CHECK(played[0].param2 == 0x4080);
    }
  }
}

TEST_CASE("Sixteen overlapping rows survive and reset cancels pending events") {
  DelayedRows rows;
  std::vector<uint8_t> played;
  for (uint8_t tick = 0; tick < 32; ++tick) {
    rows.Add(Row(tick, 15));
    rows.Tick([&](const DelayedRow &row) { played.push_back(row.note); });
  }
  REQUIRE(played.size() == 17);
  for (uint8_t note = 0; note < 17; ++note) {
    CHECK(played[note] == note);
  }
  rows.Clear();
  rows.Add(Row(100, 0));
  rows.Tick([&](const DelayedRow &row) { CHECK(row.note == 100); });
  rows.Tick([](const DelayedRow &) { FAIL("cleared rows must not trigger"); });
}

TEST_CASE("Column two DLY wins and later events can expire first") {
  DelayedRows rows;
  DelayedRow first = Row(60, 15);
  first.cmd2 = TrackerCommand::InstrumentCommandDelay;
  first.param2 = 2;
  rows.Add(first);
  rows.Tick([](const DelayedRow &) { FAIL("too early"); });
  rows.Add(Row(61, 1));
  rows.Add(Row(62, 0));
  std::vector<uint8_t> played;
  rows.Tick([&](const DelayedRow &row) { played.push_back(row.note); });
  REQUIRE(played.size() == 1);
  CHECK(played[0] == 62);
  rows.Tick([&](const DelayedRow &row) { played.push_back(row.note); });
  REQUIRE(played.size() == 3);
  CHECK(played[1] == 60);
  CHECK(played[2] == 61);
}

TEST_CASE("A last-row event retains its data after a phrase transition") {
  DelayedRows rows;
  DelayedRow source = Row(60, 6);
  rows.Add(source);
  rows.Tick([](const DelayedRow &) { FAIL("too early"); });
  // The sequencer enters a new phrase, with different note, commands and
  // chain transpose. Editing the original row also cannot alter the snapshot.
  source.note = 72;
  source.transpose = 24;
  source.param2 = 0;
  rows.Add(source);
  uint8_t count = 0;
  for (uint8_t tick = 1; tick <= 7; ++tick) {
    rows.Tick([&](const DelayedRow &row) {
      if (count++ == 0) {
        CHECK(tick == 6);
        CHECK(row.note == 60);
        CHECK(row.transpose == 12);
        CHECK(row.param2 == 0x4080);
      } else {
        CHECK(tick == 7);
        CHECK(row.note == 72);
        CHECK(row.transpose == 24);
      }
    });
  }
  CHECK(count == 2);
}
