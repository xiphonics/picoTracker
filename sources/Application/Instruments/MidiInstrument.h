/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _MIDI_INSTRUMENT_H_
#define _MIDI_INSTRUMENT_H_

#include "Application/Model/Song.h"
#include "Application/Persistency/PersistenceConstants.h"
#include "Externals/etl/include/etl/string.h"
#include "I_Instrument.h"
#include "Services/Midi/MidiMessage.h"
#include "Services/Midi/MidiService.h"

#define MAX_MIDI_CHORD_NOTES 4
#define INITIAL_NOTE_VELOCITY 0x7F

// Constants for MIDI pitch bend.
#define PB_MAX_GROWTH_FACTOR 1.5f
#define PB_MIN_GROWTH_FACTOR 1.0001f
#define PB_CENTER 8192
#define PB_MAX 16383
#define PB_7BIT_MAX 127

class MidiInstrument : public I_Instrument {

public:
  MidiInstrument();
  virtual ~MidiInstrument();

  virtual bool Init();

  // Start & stop the instument
  virtual bool Start(int channel, unsigned char note, bool retrigger = true);
  virtual void Stop(int channel);

  // size refers to the number of samples
  // should always fill interleaved stereo / 16bit
  virtual bool Render(int channel, fixed *buffer, int size, bool updateTick);
  virtual void ProcessCommand(int channel, FourCC cc, ushort value);

  virtual bool IsInitialized();

  virtual bool IsEmpty() { return false; };

  virtual InstrumentType GetType() { return IT_MIDI; };

  virtual etl::string<MAX_INSTRUMENT_NAME_LENGTH> GetDefaultName();

  virtual void OnStart();

  virtual void Purge(){};

  virtual int GetTable();
  virtual bool GetTableAutomation();
  virtual void GetTableState(TableSaveState &state);
  virtual void SetTableState(TableSaveState &state);
  etl::ilist<Variable *> *Variables() { return &variables_; };

  void SetChannel(int i);
  void SendProgramChange(int channel, int program);
  void SendProgramChangeWithNote(int channel, int program);

  // Static callback for handling delayed note-off messages
  static void NoteOffCallback();

  // Structure to hold note-off information
  struct NoteOffInfo {
    int channel;
    uint8_t note;
    static NoteOffInfo current;
  };

private:
  etl::list<Variable *, 7> variables_;

  etl::array<uint8_t, MAX_MIDI_CHORD_NOTES + 1> lastNotes_[SONG_CHANNEL_COUNT];
  int remainingTicks_;
  bool playing_;
  bool retrig_;
  int retrigLoop_;
  char velocity_ = 127;
  TableSaveState tableState_;
  bool first_[SONG_CHANNEL_COUNT];
  uint8_t pitchBendTarget_;
  uint8_t pitchBendSpeed_;
  float pitchBendCurrent_;
  float pitchBendStep_;
  float growthFactor_ = 1.0f;
  bool pitchBend_;
  bool useLogCurve_;

  Variable channel_;
  Variable noteLen_;
  Variable volume_;
  Variable table_;
  Variable tableAuto_;
  Variable program_;
  // need to store defaultname as it depends on the MIDI channel of the
  // instrument
  etl::string<MAX_INSTRUMENT_NAME_LENGTH> defaultName_;

  static MidiService *svc_;
  static TimerService *timerSvc_;
};

#endif
