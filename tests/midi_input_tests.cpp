#include "Adapters/picoTracker/midi/picoTrackerUSBMidiInDevice.h"
#include "Application/Player/Player.h"
#include "doctest/doctest.h"
#include <array>
#include <deque>

namespace {
std::deque<std::array<uint8_t, 4>> packets;

class Input : public MidiInDevice {
public:
  Input() : MidiInDevice("test") {}
  bool Start() override { return MidiInDevice::Start(); }
  void Stop() override { MidiInDevice::Stop(); }
  void poll() override {}
  void Feed(uint8_t byte) { processMidiData(byte); }
  bool driverStarts = true;

protected:
  bool initDriver() override { return true; }
  void closeDriver() override {}
  bool startDriver() override { return driverStarts; }
  void stopDriver() override {}
};
} // namespace

bool tud_midi_mounted() { return true; }
uint32_t tud_midi_available() { return packets.size(); }
bool tud_midi_packet_read(uint8_t *packet) {
  if (packets.empty()) {
    return false;
  }
  std::copy(packets.front().begin(), packets.front().end(), packet);
  packets.pop_front();
  return true;
}

TEST_CASE("Disabled MIDI input ignores notes and transport") {
  *Player::GetInstance() = Player{};
  Input input;
  input.Feed(0x90);
  input.Feed(60);
  input.Feed(100);
  input.Feed(0xFA);
  CHECK(Player::GetInstance()->notesPlayed == 0);
  CHECK(Player::GetInstance()->starts == 0);
  REQUIRE(input.Start());
  input.Feed(0x90);
  input.Feed(60);
  input.Feed(100);
  input.Feed(0xFA);
  CHECK(Player::GetInstance()->notesPlayed == 1);
  CHECK(Player::GetInstance()->starts == 1);
  input.Stop();
  CHECK_FALSE(input.IsRunning());
  CHECK(Player::GetInstance()->notesStopped == 1);
  input.Feed(0xFC);
  CHECK(Player::GetInstance()->stops == 0);
}

TEST_CASE("Re-enabling input drops partial messages and running status") {
  *Player::GetInstance() = Player{};
  Input input;
  REQUIRE(input.Start());
  input.Feed(0x90);
  input.Feed(60);
  input.Stop();
  REQUIRE(input.Start());
  input.Feed(100);
  CHECK(Player::GetInstance()->notesPlayed == 0);
  input.Feed(0x90);
  input.Feed(62);
  input.Feed(100);
  CHECK(Player::GetInstance()->notesPlayed == 1);
  input.Stop();
  CHECK(Player::GetInstance()->notesStopped == 1);
  REQUIRE(input.Start());
  input.Feed(63);
  input.Feed(100);
  CHECK(Player::GetInstance()->notesPlayed == 1);
}

TEST_CASE("Failed driver start leaves MIDI input disabled") {
  Input input;
  input.driverStarts = false;
  CHECK_FALSE(input.Start());
  CHECK_FALSE(input.IsRunning());
}

TEST_CASE(
    "USB input drains disabled traffic and drops pending packets on enable") {
  *Player::GetInstance() = Player{};
  packets.clear();
  picoTrackerUSBMidiInDevice input("USB test");
  packets.push_back({0x09, 0x90, 60, 100});
  packets.push_back({0x0F, 0xFA, 0, 0});
  input.poll();
  CHECK(packets.empty());
  CHECK(Player::GetInstance()->notesPlayed == 0);
  CHECK(Player::GetInstance()->starts == 0);
  packets.push_back({0x09, 0x90, 61, 100});
  REQUIRE(input.Start());
  CHECK(packets.empty());
  input.poll();
  CHECK(Player::GetInstance()->notesPlayed == 0);
  packets.push_back({0x09, 0x90, 62, 100});
  input.poll();
  CHECK(Player::GetInstance()->notesPlayed == 1);
  input.Stop();
  CHECK(Player::GetInstance()->notesStopped == 1);
  packets.push_back({0x09, 0x90, 63, 100});
  input.poll();
  CHECK(packets.empty());
  CHECK(Player::GetInstance()->notesPlayed == 1);
}
