#include "Application/Model/Project.h"
#include "Application/Player/RowFx.h"
#include "System/System/System.h"
#include "doctest/doctest.h"
#include <array>
#include <initializer_list>

namespace {

class TestSystem : public System {
public:
  unsigned long GetClock() override { return 0; }
  void GetBatteryState(BatteryState &state) override {
    state = {
        .percentage = 0,
        .voltage_mv = 0,
        .temperature_c = 0,
        .charging = false,
        .error = false,
    };
  }
  void SetDisplayBrightness(unsigned char) override {}
  void PostQuitMessage() override {}
  unsigned int GetMemoryUsage() override { return 0; }
  void PowerDown() override {}
  void SystemPutChar(int) override {}
  void SystemBootloader() override {}
  void SystemReboot() override {}
  uint32_t GetRandomNumber() override {
    if (randomNumberIndex_ < randomNumberCount_) {
      return randomNumbers_[randomNumberIndex_++];
    }
    return randomNumber_;
  }
  uint32_t Micros() override { return 0; }
  uint32_t Millis() override { return 0; }

  void SetRandomNumbers(std::initializer_list<uint32_t> randomNumbers) {
    randomNumberCount_ = 0;
    randomNumberIndex_ = 0;
    for (uint32_t randomNumber : randomNumbers) {
      randomNumbers_[randomNumberCount_++] = randomNumber;
    }
  }

  uint32_t randomNumber_ = 0;
  uint32_t randomNumbers_[16] = {};
  uint8_t randomNumberCount_ = 0;
  uint8_t randomNumberIndex_ = 0;
};

class SystemInstallGuard {
public:
  explicit SystemInstallGuard(System *system) { System::Install(system); }
  ~SystemInstallGuard() { System::Install(nullptr); }
};

} // namespace

TEST_CASE("Table CHN in column 2 gates table column 1") {
  TestSystem system;
  SystemInstallGuard installGuard(&system);

  const TrackerCommand command[TABLE_COLUMNS] = {
      TrackerCommand::InstrumentCommandVolume,
      TrackerCommand::InstrumentCommandChance,
      TrackerCommand::InstrumentCommandNone,
  };
  const ushort param[TABLE_COLUMNS] = {0x4080, 0x0000, 0};
  const PlayerLastActiveCommandState lastActive = {
      .lastCmd = TrackerCommand::InstrumentCommandNone,
      .lastParam = 0,
      .valid = false,
  };

  const TableRowFxResult fx =
      ResolveTableRowFx(command, param, 60, lastActive, true);

  CHECK_FALSE(fx.allowCmd[0]);
  CHECK(fx.allowCmd[1]);
  CHECK(fx.allowCmd[2]);
  CHECK(fx.cmd[0] == TrackerCommand::InstrumentCommandVolume);
  CHECK(fx.param[0] == 0x4080);
}

TEST_CASE("Table CHN 01 gates the current note target") {
  TestSystem system;
  SystemInstallGuard installGuard(&system);

  const TrackerCommand command[TABLE_COLUMNS] = {
      TrackerCommand::InstrumentCommandChance,
      TrackerCommand::InstrumentCommandVolume,
      TrackerCommand::InstrumentCommandPan,
  };
  const ushort param[TABLE_COLUMNS] = {0x0100, 0x4080, 0x2080};
  const PlayerLastActiveCommandState lastActive = {
      .lastCmd = TrackerCommand::InstrumentCommandNone,
      .lastParam = 0,
      .valid = false,
  };

  const TableRowFxResult fx =
      ResolveTableRowFx(command, param, 60, lastActive, true);

  CHECK_FALSE(fx.allowNote);
  CHECK(fx.allowCmd[0]);
  CHECK(fx.allowCmd[1]);
  CHECK(fx.allowCmd[2]);
}

TEST_CASE("Phrase RND in column 1 randomizes the note target") {
  TestSystem system;
  SystemInstallGuard installGuard(&system);
  system.randomNumber_ = 0xFF;

  const PlayerLastActiveCommandState lastActive = {
      .lastCmd = TrackerCommand::InstrumentCommandNone,
      .lastParam = 0,
      .valid = false,
  };

  const RowFxResult fx =
      ResolveRowFx(60, TrackerCommand::InstrumentCommandRandom, 0x00FF,
                   TrackerCommand::InstrumentCommandNone, 0, lastActive, false);

  CHECK(fx.allowNote);
  CHECK(fx.note == HIGHEST_NOTE);
}

TEST_CASE("Phrase RND in column 2 randomizes command column 1") {
  TestSystem system;
  SystemInstallGuard installGuard(&system);
  system.randomNumber_ = 0x00;

  const PlayerLastActiveCommandState lastActive = {
      .lastCmd = TrackerCommand::InstrumentCommandVolume,
      .lastParam = 0x4080,
      .valid = true,
  };

  const RowFxResult fx = ResolveRowFx(
      60, TrackerCommand::InstrumentCommandVolume, 0x4080,
      TrackerCommand::InstrumentCommandRandom, 0x00FF, lastActive, true);

  CHECK(fx.cmd1 == TrackerCommand::InstrumentCommandVolume);
  CHECK(fx.param1 == 0x4000);
  CHECK(fx.cmd2 == TrackerCommand::InstrumentCommandRandom);
}

TEST_CASE("Phrase CHN can gate RND before it randomizes its note target") {
  TestSystem system;
  SystemInstallGuard installGuard(&system);
  system.SetRandomNumbers({0xFF, 0xFF});

  const PlayerLastActiveCommandState lastActive = {
      .lastCmd = TrackerCommand::InstrumentCommandVolume,
      .lastParam = 0x4080,
      .valid = true,
  };

  const RowFxResult fx = ResolveRowFx(
      60, TrackerCommand::InstrumentCommandRandom, 0x00FF,
      TrackerCommand::InstrumentCommandChance, 0x0000, lastActive, false);

  CHECK_FALSE(fx.allowCmd1);
  CHECK(fx.allowCmd2);
  CHECK(fx.note == 60);
}

TEST_CASE("RND byte fields only affect matching target bytes") {
  TestSystem system;
  SystemInstallGuard installGuard(&system);
  system.randomNumber_ = 0xFF;

  const PlayerLastActiveCommandState lastActive = {
      .lastCmd = TrackerCommand::InstrumentCommandNone,
      .lastParam = 0,
      .valid = false,
  };

  const RowFxResult fx = ResolveRowFx(
      60, TrackerCommand::InstrumentCommandVolume, 0x4080,
      TrackerCommand::InstrumentCommandRandom, 0x0010, lastActive, true);

  CHECK(fx.param1 == 0x4090);
}

TEST_CASE("RND randomizes ARP as four independent nibbles") {
  TestSystem system;
  SystemInstallGuard installGuard(&system);
  system.SetRandomNumbers({0xFF, 0x00, 0xFF, 0x00});

  const PlayerLastActiveCommandState lastActive = {
      .lastCmd = TrackerCommand::InstrumentCommandNone,
      .lastParam = 0,
      .valid = false,
  };

  const RowFxResult fx = ResolveRowFx(
      60, TrackerCommand::InstrumentCommandArpeggiator, 0x8888,
      TrackerCommand::InstrumentCommandRandom, 0xFFFF, lastActive, true);

  CHECK(fx.param1 == 0xF0F0);
}

TEST_CASE("RND randomizes CSH drive and crush nibble only") {
  TestSystem system;
  SystemInstallGuard installGuard(&system);
  system.SetRandomNumbers({0xFF, 0xFF});

  const PlayerLastActiveCommandState lastActive = {
      .lastCmd = TrackerCommand::InstrumentCommandNone,
      .lastParam = 0,
      .valid = false,
  };

  const RowFxResult fx = ResolveRowFx(
      60, TrackerCommand::InstrumentCommandCrush, 0x80F8,
      TrackerCommand::InstrumentCommandRandom, 0xFF0F, lastActive, true);

  CHECK(fx.param1 == 0xFF0F);
}

TEST_CASE("RND leaves MIDI CC number unchanged and randomizes value") {
  TestSystem system;
  SystemInstallGuard installGuard(&system);
  system.randomNumber_ = 0xFF;

  const PlayerLastActiveCommandState lastActive = {
      .lastCmd = TrackerCommand::InstrumentCommandNone,
      .lastParam = 0,
      .valid = false,
  };

  const RowFxResult fx = ResolveRowFx(
      60, TrackerCommand::InstrumentCommandMidiCC, 0x1040,
      TrackerCommand::InstrumentCommandRandom, 0x00FF, lastActive, true);

  CHECK(fx.param1 == 0x107F);
}

TEST_CASE("RND randomizes HOP during the normal command pass") {
  TestSystem system;
  SystemInstallGuard installGuard(&system);
  system.randomNumber_ = 0xFF;

  const PlayerLastActiveCommandState lastActive = {
      .lastCmd = TrackerCommand::InstrumentCommandNone,
      .lastParam = 0,
      .valid = false,
  };

  const RowFxResult fx = ResolveRowFx(
      60, TrackerCommand::InstrumentCommandHop, 0x1204,
      TrackerCommand::InstrumentCommandRandom, 0x00FF, lastActive, true);

  CHECK(fx.param1 == 0x120F);
}

TEST_CASE("ResolveRowHop uses the randomized HOP destination") {
  TestSystem system;
  SystemInstallGuard installGuard(&system);
  system.randomNumber_ = 0xFF;

  const int32_t hop =
      ResolveRowHop(TrackerCommand::InstrumentCommandHop, 0x1204,
                    TrackerCommand::InstrumentCommandRandom, 0x00FF);

  CHECK(hop == 0x0F);
}

TEST_CASE("ResolveRowHop respects CHN gating") {
  TestSystem system;
  SystemInstallGuard installGuard(&system);
  system.randomNumber_ = 0x00;

  SUBCASE("Gating HOP in column 1 with CHN in column 2") {
    const int32_t hop =
        ResolveRowHop(TrackerCommand::InstrumentCommandHop, 0x1204,
                      TrackerCommand::InstrumentCommandChance, 0x0000);
    CHECK(hop == -1);
  }

  SUBCASE("Allowing HOP in column 1 with CHN in column 2") {
    const int32_t hop =
        ResolveRowHop(TrackerCommand::InstrumentCommandHop, 0x1204,
                      TrackerCommand::InstrumentCommandChance, 0x00FF);
    CHECK(hop == 0x04);
  }
}

TEST_CASE("ResolveRowHop randomizes both repeat count and destination") {
  TestSystem system;
  SystemInstallGuard installGuard(&system);
  system.randomNumber_ = 0xFF;

  SUBCASE("Randomizes only destination when high byte of RND is 00") {
    const int32_t hop =
        ResolveRowHop(TrackerCommand::InstrumentCommandHop, 0x1204,
                      TrackerCommand::InstrumentCommandRandom, 0x00FF);
    CHECK(hop == 0x0F);
  }

  SUBCASE("Randomizes repeat count as well") {
    const PlayerLastActiveCommandState lastActive = {
        .lastCmd = TrackerCommand::InstrumentCommandNone,
        .lastParam = 0,
        .valid = false,
    };
    RowFxResult fx =
        ResolveRowFx(NO_NOTE, TrackerCommand::InstrumentCommandHop, 0x1204,
                     TrackerCommand::InstrumentCommandRandom, 0x1080,
                     lastActive, true, false);
    CHECK(fx.param1 == 0x220C);
  }
}

TEST_CASE("ResolveRowHop uses the full low byte as RND depth") {
  TestSystem system;
  SystemInstallGuard installGuard(&system);
  system.randomNumber_ = 0xFF;

  const int32_t hop =
      ResolveRowHop(TrackerCommand::InstrumentCommandHop, 0x1204,
                    TrackerCommand::InstrumentCommandRandom, 0x0080);

  CHECK(hop == 0x0C);
}

TEST_CASE("ResolveRowHop avoids clamping bias at the low edge") {
  TestSystem system;
  SystemInstallGuard installGuard(&system);
  system.randomNumber_ = 0xFF;

  const int32_t hop =
      ResolveRowHop(TrackerCommand::InstrumentCommandHop, 0x0000,
                    TrackerCommand::InstrumentCommandRandom, 0x00E0);

  CHECK(hop == 0x0D);
}

TEST_CASE("RND preserves GRV scope flag and randomizes groove value") {
  TestSystem system;
  SystemInstallGuard installGuard(&system);
  system.randomNumber_ = 0xFF;

  const PlayerLastActiveCommandState lastActive = {
      .lastCmd = TrackerCommand::InstrumentCommandNone,
      .lastParam = 0,
      .valid = false,
  };

  const RowFxResult fx = ResolveRowFx(
      60, TrackerCommand::InstrumentCommandGroove, 0x0140,
      TrackerCommand::InstrumentCommandRandom, 0x00FF, lastActive, true);

  CHECK(fx.param1 == 0x01FF);
}

TEST_CASE("RND randomizes tempo as one clamped whole value") {
  TestSystem system;
  SystemInstallGuard installGuard(&system);
  system.randomNumber_ = 0xFF;

  const PlayerLastActiveCommandState lastActive = {
      .lastCmd = TrackerCommand::InstrumentCommandNone,
      .lastParam = 0,
      .valid = false,
  };

  const RowFxResult fx = ResolveRowFx(
      60, TrackerCommand::InstrumentCommandTempo, 0x012C,
      TrackerCommand::InstrumentCommandRandom, 0x00FF, lastActive, true);

  CHECK(fx.param1 == MAX_TEMPO);
}

TEST_CASE("RND does not randomize protected target commands") {
  TestSystem system;
  SystemInstallGuard installGuard(&system);
  system.randomNumber_ = 0xFF;

  const PlayerLastActiveCommandState lastActive = {
      .lastCmd = TrackerCommand::InstrumentCommandNone,
      .lastParam = 0,
      .valid = false,
  };

  const RowFxResult fx = ResolveRowFx(
      60, TrackerCommand::InstrumentCommandTable, 0x0040,
      TrackerCommand::InstrumentCommandRandom, 0x00FF, lastActive, true);

  CHECK(fx.param1 == 0x0040);
}

TEST_CASE("Table RND in column 1 randomizes the current note target") {
  TestSystem system;
  SystemInstallGuard installGuard(&system);
  system.randomNumber_ = 0xFF;

  const TrackerCommand command[TABLE_COLUMNS] = {
      TrackerCommand::InstrumentCommandRandom,
      TrackerCommand::InstrumentCommandNone,
      TrackerCommand::InstrumentCommandNone,
  };
  const ushort param[TABLE_COLUMNS] = {0x00FF, 0, 0};
  const PlayerLastActiveCommandState lastActive = {
      .lastCmd = TrackerCommand::InstrumentCommandNone,
      .lastParam = 0,
      .valid = false,
  };

  const TableRowFxResult fx =
      ResolveTableRowFx(command, param, 60, lastActive, true);

  CHECK(fx.allowNote);
  CHECK(fx.note == HIGHEST_NOTE);
}

TEST_CASE("Table CHN can gate RND before it randomizes its target column") {
  TestSystem system;
  SystemInstallGuard installGuard(&system);
  system.SetRandomNumbers({0xFF, 0xFF});

  const TrackerCommand command[TABLE_COLUMNS] = {
      TrackerCommand::InstrumentCommandVolume,
      TrackerCommand::InstrumentCommandRandom,
      TrackerCommand::InstrumentCommandChance,
  };
  const ushort param[TABLE_COLUMNS] = {0x4080, 0x00FF, 0x0000};
  const PlayerLastActiveCommandState lastActive = {
      .lastCmd = TrackerCommand::InstrumentCommandNone,
      .lastParam = 0,
      .valid = false,
  };

  const TableRowFxResult fx =
      ResolveTableRowFx(command, param, 60, lastActive, true);

  CHECK_FALSE(fx.allowCmd[1]);
  CHECK(fx.allowCmd[2]);
  CHECK(fx.param[0] == 0x4080);
}

TEST_CASE("RND covers every Sample instrument command shape") {
  TestSystem system;
  SystemInstallGuard installGuard(&system);
  system.randomNumber_ = 0xFF;

  const PlayerLastActiveCommandState lastActive = {
      .lastCmd = TrackerCommand::InstrumentCommandNone,
      .lastParam = 0,
      .valid = false,
  };

  const std::array<TrackerCommand, 13> commands = {
      TrackerCommand::InstrumentCommandLoopOfset,
      TrackerCommand::InstrumentCommandPlayOfset,
      TrackerCommand::InstrumentCommandArpeggiator,
      TrackerCommand::InstrumentCommandVolume,
      TrackerCommand::InstrumentCommandPan,
      TrackerCommand::InstrumentCommandFilterCut,
      TrackerCommand::InstrumentCommandFilterResonance,
      TrackerCommand::InstrumentCommandPitchSlide,
      TrackerCommand::InstrumentCommandLegato,
      TrackerCommand::InstrumentCommandPitchFineTune,
      TrackerCommand::InstrumentCommandRetrigger,
      TrackerCommand::InstrumentCommandLowPassFilter,
      TrackerCommand::InstrumentCommandCrush,
  };
  const std::array<ushort, 13> params = {
      0x7F00, 0x4040, 0x1234, 0x4080, 0x4080, 0x2080, 0x2080,
      0x207F, 0x2002, 0x2080, 0x0402, 0x8080, 0x80F8,
  };

  for (size_t i = 0; i < commands.size(); ++i) {
    const RowFxResult fx = ResolveRowFx(60, commands[i], params[i],
                                        TrackerCommand::InstrumentCommandRandom,
                                        0xFFFF, lastActive, true);

    CHECK(fx.cmd1 == commands[i]);
    CHECK(fx.param1 != params[i]);
  }
}

TEST_CASE("CHN can gate every Sample instrument command") {
  TestSystem system;
  SystemInstallGuard installGuard(&system);
  system.randomNumber_ = 0xFF;

  const PlayerLastActiveCommandState lastActive = {
      .lastCmd = TrackerCommand::InstrumentCommandNone,
      .lastParam = 0,
      .valid = false,
  };

  const std::array<TrackerCommand, 13> commands = {
      TrackerCommand::InstrumentCommandLoopOfset,
      TrackerCommand::InstrumentCommandPlayOfset,
      TrackerCommand::InstrumentCommandArpeggiator,
      TrackerCommand::InstrumentCommandVolume,
      TrackerCommand::InstrumentCommandPan,
      TrackerCommand::InstrumentCommandFilterCut,
      TrackerCommand::InstrumentCommandFilterResonance,
      TrackerCommand::InstrumentCommandPitchSlide,
      TrackerCommand::InstrumentCommandLegato,
      TrackerCommand::InstrumentCommandPitchFineTune,
      TrackerCommand::InstrumentCommandRetrigger,
      TrackerCommand::InstrumentCommandLowPassFilter,
      TrackerCommand::InstrumentCommandCrush,
  };

  for (TrackerCommand command : commands) {
    const RowFxResult fx = ResolveRowFx(60, command, 0x4080,
                                        TrackerCommand::InstrumentCommandChance,
                                        0x0000, lastActive, true);

    CHECK_FALSE(fx.allowCmd1);
  }
}
