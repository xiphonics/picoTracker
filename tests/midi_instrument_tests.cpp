/* SPDX-License-Identifier: BSD-3-Clause */
#include "Application/Instruments/MidiInstrument.h"
#include "Application/Player/PlayerChannel.h"
#include "doctest/doctest.h"
#include <vector>

extern std::vector<MidiMessage> queuedMidiMessages;

namespace {
struct MidiFixture {
  // Keep the service installed for the lifetime of MidiInstrument's cached
  // service pointer, including across test cases.
  static MidiService &Service() {
    static MidiService service;
    MidiService::Install(&service);
    return service;
  }
  MidiService &service = Service();
  MidiInstrument instrument;

  MidiFixture() {
    queuedMidiMessages.clear();
    instrument.Init();
  }
  void Render() { instrument.Render(0, nullptr, 0, false); }
  void Start(uint8_t note, bool retrigger = true) {
    REQUIRE(instrument.Start(0, note, retrigger));
    Render();
  }
};
void CheckMessage(size_t index, uint8_t status, uint8_t note) {
  REQUIRE(index < queuedMidiMessages.size());
  CHECK(queuedMidiMessages[index].status_ == status);
  CHECK(queuedMidiMessages[index].data1_ == note);
}
} // namespace

TEST_CASE("Tied MIDI notes send the new note on before the old note off") {
  MidiFixture fixture;
  fixture.Start(60);
  queuedMidiMessages.clear();
  fixture.Start(62, false);
  REQUIRE(queuedMidiMessages.size() == 2);
  CheckMessage(0, MidiMessage::MIDI_NOTE_ON, 62);
  CHECK(queuedMidiMessages[0].data2_ == INITIAL_NOTE_VELOCITY);
  CheckMessage(1, MidiMessage::MIDI_NOTE_OFF, 60);
}

TEST_CASE("Same-pitch MIDI ties extend the note length without retriggering") {
  MidiFixture fixture;
  fixture.instrument.FindVariable(FourCC::MidiInstrumentNoteLength)
      ->SetInt(3, false);
  fixture.Start(60);
  fixture.Render();
  queuedMidiMessages.clear();
  fixture.Start(60, false);
  fixture.Render();
  CHECK(queuedMidiMessages.empty());
  fixture.Render();
  REQUIRE(queuedMidiMessages.size() == 1);
  CheckMessage(0, MidiMessage::MIDI_NOTE_OFF, 60);
}

TEST_CASE("MIDI ties release chord tones and overlap only the root note") {
  MidiFixture fixture;
  fixture.Start(60);
  fixture.instrument.ProcessCommand(
      0, TrackerCommand::InstrumentCommandMidiChord, 0x0301);
  queuedMidiMessages.clear();
  fixture.Start(65, false);
  REQUIRE(queuedMidiMessages.size() == 4);
  CheckMessage(0, MidiMessage::MIDI_NOTE_OFF, 61);
  CheckMessage(1, MidiMessage::MIDI_NOTE_OFF, 63);
  CheckMessage(2, MidiMessage::MIDI_NOTE_ON, 65);
  CheckMessage(3, MidiMessage::MIDI_NOTE_OFF, 60);
  queuedMidiMessages.clear();
  fixture.instrument.Stop(0);
  REQUIRE(queuedMidiMessages.size() == 1);
  CheckMessage(0, MidiMessage::MIDI_NOTE_OFF, 65);
}

TEST_CASE("Stopping a pending MIDI tie clears it and supports MIDI note zero") {
  MidiFixture fixture;
  fixture.Start(0);
  queuedMidiMessages.clear();
  REQUIRE(fixture.instrument.Start(0, 62, false));
  fixture.instrument.Stop(0);
  REQUIRE(queuedMidiMessages.size() == 2);
  CheckMessage(0, MidiMessage::MIDI_NOTE_OFF, 0);
  CheckMessage(1, MidiMessage::MIDI_NOTE_OFF, 62);
  queuedMidiMessages.clear();
  fixture.Render();
  CHECK(queuedMidiMessages.empty());
  fixture.Start(64, false);
  REQUIRE(queuedMidiMessages.size() == 1);
  CheckMessage(0, MidiMessage::MIDI_NOTE_ON, 64);
}

TEST_CASE("PlayerChannel preserves a tied MIDI instrument and retriggers "
          "explicit notes") {
  MidiFixture fixture;
  PlayerChannel channel(0);
  channel.StartInstrument(&fixture.instrument, 60, true);
  channel.Render(nullptr, 0);
  queuedMidiMessages.clear();
  channel.StartInstrument(&fixture.instrument, 62, false);
  channel.Render(nullptr, 0);
  REQUIRE(queuedMidiMessages.size() == 2);
  CheckMessage(0, MidiMessage::MIDI_NOTE_ON, 62);
  CheckMessage(1, MidiMessage::MIDI_NOTE_OFF, 60);
  queuedMidiMessages.clear();
  channel.StartInstrument(&fixture.instrument, 64, true);
  channel.Render(nullptr, 0);
  REQUIRE(queuedMidiMessages.size() == 2);
  CheckMessage(0, MidiMessage::MIDI_NOTE_OFF, 62);
  CheckMessage(1, MidiMessage::MIDI_NOTE_ON, 64);
  channel.StopInstrument();
}

TEST_CASE("PlayerChannel stops the old instrument when switching instruments") {
  MidiFixture fixture;
  MidiInstrument otherInstrument;
  otherInstrument.SetChannel(1);
  PlayerChannel channel(0);
  channel.StartInstrument(&fixture.instrument, 60, true);
  channel.Render(nullptr, 0);
  queuedMidiMessages.clear();
  channel.StartInstrument(&otherInstrument, 62, false);
  channel.Render(nullptr, 0);
  REQUIRE(queuedMidiMessages.size() == 2);
  CheckMessage(0, MidiMessage::MIDI_NOTE_OFF, 60);
  CheckMessage(1, MidiMessage::MIDI_NOTE_ON + 1, 62);
  channel.StopInstrument();
}
