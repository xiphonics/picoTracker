#include "doctest/doctest.h"

#include "Application/Instruments/CommandList.h"
#include "Application/Model/Phrase.h"
#include "Foundation/Types/Types.h"
#include <cstring>
#include <type_traits>

TEST_CASE("TrackerCommand stable persisted enum values") {
  CHECK(TrackerCommand::InstrumentCommandArpeggiator == 0);
  CHECK(TrackerCommand::InstrumentCommandCrush == 2);
  CHECK(TrackerCommand::InstrumentCommandDelay == 4);
  CHECK(TrackerCommand::InstrumentCommandFilterCut == 20);
  CHECK(TrackerCommand::InstrumentCommandLowPassFilter == 22);
  CHECK(TrackerCommand::InstrumentCommandFilterResonance == 25);
  CHECK(TrackerCommand::InstrumentCommandGateOff == 92);
  CHECK(TrackerCommand::InstrumentCommandGroove == 26);
  CHECK(TrackerCommand::InstrumentCommandHop == 27);
  CHECK(TrackerCommand::InstrumentCommandRetrigger == 52);
  CHECK(TrackerCommand::InstrumentCommandInstrumentRetrigger == 29);
  CHECK(TrackerCommand::InstrumentCommandKill == 30);
  CHECK(TrackerCommand::InstrumentCommandLegato == 31);
  CHECK(TrackerCommand::InstrumentCommandLoopOfset == 36);
  CHECK(TrackerCommand::InstrumentCommandMidiCC == 38);
  CHECK(TrackerCommand::InstrumentCommandMidiPC == 39);
  CHECK(TrackerCommand::InstrumentCommandPan == 42);
  CHECK(TrackerCommand::InstrumentCommandPitchFineTune == 44);
  CHECK(TrackerCommand::InstrumentCommandPlayOfset == 46);
  CHECK(TrackerCommand::InstrumentCommandPitchSlide == 48);
  CHECK(TrackerCommand::InstrumentCommandStop == 55);
  CHECK(TrackerCommand::InstrumentCommandTable == 58);
  CHECK(TrackerCommand::InstrumentCommandTempo == 62);
  CHECK(TrackerCommand::InstrumentCommandVelocity == 66);
  CHECK(TrackerCommand::InstrumentCommandVolume == 69);
  CHECK(TrackerCommand::InstrumentCommandNone == 45);
  CHECK(TrackerCommand::InstrumentCommandMidiChord == 143);
  CHECK(TrackerCommand::InstrumentCommandChance == 188);
  CHECK(TrackerCommand::InstrumentCommandRandom == 189);
}

TEST_CASE("Commands retain byte storage and unsigned values") {
  CHECK(sizeof(TrackerCommand) == 1);
  CHECK(sizeof(TrackerCommand::enum_type) == 1);
  CHECK(std::is_same_v<TrackerCommand::value_type, uint8_t>);
  CHECK(sizeof(Phrase::cmd1_) == PHRASE_COUNT * STEPS_PER_PHRASE);
  CHECK(sizeof(Phrase::cmd2_) == PHRASE_COUNT * STEPS_PER_PHRASE);

  TrackerCommand commands[] = {TrackerCommand::InstrumentCommandArpeggiator,
                               TrackerCommand::InstrumentCommandNone,
                               TrackerCommand::InstrumentCommandMidiChord,
                               TrackerCommand::Default};
  const uint8_t savedBytes[] = {0, 45, 143, 255};
  CHECK(std::memcmp(commands, savedBytes, sizeof(savedBytes)) == 0);
  CHECK(commands[2].get_value() == 143);
  CHECK(commands[3].get_value() == 255);
  CHECK(std::strcmp(commands[2].c_str(), "MCH") == 0);
}

TEST_CASE("Runtime FourCC supports identifiers above 255") {
  CHECK(sizeof(FourCC) == 2);
  CHECK(std::is_same_v<FourCC::value_type, uint16_t>);
  FourCC id(static_cast<FourCC::value_type>(300));
  CHECK(id.get_value() == 300);
  CHECK(FourCC::VarChannel8Volume == FourCC::VarChannel1Volume + 7);
  CHECK(FourCC::VarOutputVolume != FourCC::ActionShowSampleSlices);
  CHECK(std::strcmp(FourCC(FourCC::VarTempo).c_str(), "tempo") == 0);
  CHECK(std::strcmp(FourCC(FourCC::VarScaleRoot).c_str(), "scaleroot") == 0);
  CHECK(std::strcmp(FourCC(FourCC::OPALInstrumentOp2Level).c_str(),
                    "OP2LEVEL") == 0);
  CHECK(std::strcmp(FourCC(FourCC::VarOutputVolume).c_str(), "outputvolume") ==
        0);
}

TEST_CASE("Command navigation retains Pico mnemonic groups") {
  CHECK(CommandList::GetNext(TrackerCommand::InstrumentCommandMidiCC) ==
        TrackerCommand::InstrumentCommandMidiChord);
  CHECK(CommandList::GetPrev(TrackerCommand::InstrumentCommandMidiPC) ==
        TrackerCommand::InstrumentCommandMidiChord);
  CHECK(CommandList::GetNextAlpha(TrackerCommand::InstrumentCommandMidiChord) ==
        TrackerCommand::InstrumentCommandPan);
  CHECK(CommandList::GetPrevAlpha(TrackerCommand::InstrumentCommandPan) ==
        TrackerCommand::InstrumentCommandMidiCC);
  CHECK(CommandList::GetNextAlpha(TrackerCommand::InstrumentCommandFilterCut) ==
        TrackerCommand::InstrumentCommandGateOff);
  CHECK(CommandList::RangeLimitCommandParam(
            TrackerCommand::InstrumentCommandVelocity, 0xABFF) == 0xAB7F);
  CHECK(CommandList::RangeLimitCommandParam(
            TrackerCommand::InstrumentCommandPan, 0xABFF) == 0xABFE);
  CHECK(CommandList::RangeLimitCommandParam(
            TrackerCommand::InstrumentCommandVolume, 0xABFF) == 0xABFF);
}
