/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 * Copyright (c) 2925 nILS Podewski
 *
 * This file is part of the picoTracker firmware
 */

#include "GameBoyInstrument.h"
#include "CommandList.h"
#include "Externals/etl/include/etl/to_string.h"
#include "I_Instrument.h"
#include "System/Console/Trace.h"
#include "System/Profiler/Profiler.h"
#include "bit.h"
#include <string.h>

static const char *algorithms[2] = {"1*2", "1+2"};

static const char *waveShapes[8] = {"sine", "half", "abs", "puls",
                                    "even", "ab-e", "sqr", "dsqr"};

static const char *kslValues[4] = {"0", "1.5", "3", "6"};

#define FREQ_BASE_REG 0xA0
#define OCTAVE_BASE_REG 0xB0

#define CHANNEL 0 // just hardcoding to channel 0 for now

static const unsigned int noteFNumbers[] = {342, 363, 385, 408, 432, 458,
                                            485, 514, 544, 577, 611, 647};

GameBoyInstrument::GameBoyInstrument()
    : I_Instrument(&variables_),
      algorithm_(FourCC::GameBoyInstrumentAlgorithm, algorithms, 6, 0),
      feedback_(FourCC::GameBoyInstrumentFeedback, 0),
      deepTremeloVibrato_(FourCC::GameBoyInstrumentDeepTremeloVibrato, 0),
      op1Level_(FourCC::GameBoyInstrumentOp1Level, 0x17),
      op1Multiplier_(FourCC::GameBoyInstrumentOp1Multiplier, 0x1),
      op1ADSR_(FourCC::GameBoyInstrumentOp1ADSR, 0xF1C8),
      op1WaveShape_(FourCC::GameBoyInstrumentOp1WaveShape, waveShapes, 8, 0),
      op1TremVibSusKSR_(FourCC::GameBoyInstrumentOp1TremVibSusKSR, 0),
      op1KeyScaleLevel_(FourCC::GameBoyInstrumentOp1KeyScaleLevel, kslValues, 4,
                        0x1),
      op2Level_(FourCC::GameBoyInstrumentOp2Level, 0),
      op2Multiplier_(FourCC::GameBoyInstrumentOp2Multiplier, 0x1),
      op2ADSR_(FourCC::GameBoyInstrumentOp2ADSR, 0xF1D8),
      op2WaveShape_(FourCC::GameBoyInstrumentOp2WaveShape, waveShapes, 8, 0),
      op2TremVibSusKSR_(FourCC::GameBoyInstrumentOp2TremVibSusKSR, 0x2),
      op2KeyScaleLevel_(FourCC::GameBoyInstrumentOp2KeyScaleLevel, kslValues, 4,
                        0) {

  // name_ is now an etl::string in the base class, not a Variable
  variables_.insert(variables_.end(), &algorithm_);
  variables_.insert(variables_.end(), &feedback_);
  variables_.insert(variables_.end(), &deepTremeloVibrato_);
  variables_.insert(variables_.end(), &op1Level_);
  variables_.insert(variables_.end(), &op1Multiplier_);
  variables_.insert(variables_.end(), &op1ADSR_);
  variables_.insert(variables_.end(), &op1WaveShape_);
  variables_.insert(variables_.end(), &op1KeyScaleLevel_);
  variables_.insert(variables_.end(), &op1TremVibSusKSR_);
  variables_.insert(variables_.end(), &op2Level_);
  variables_.insert(variables_.end(), &op2Multiplier_);
  variables_.insert(variables_.end(), &op2ADSR_);
  variables_.insert(variables_.end(), &op2WaveShape_);
  variables_.insert(variables_.end(), &op2KeyScaleLevel_);
  variables_.insert(variables_.end(), &op2TremVibSusKSR_);
}

GameBoyInstrument::~GameBoyInstrument(){};

bool GameBoyInstrument::Init() {
  // enable left/right only for 0 channel

  return true;
};

void GameBoyInstrument::OnStart(){};

bool GameBoyInstrument::Start(int channel, unsigned char note, bool retrigger) {
  // channel wide settings
  // enable left/right output (D4, D5) & set algorithm D0
  // for now only 2 op so just Additive or FM


  // set note in OPAL
  uint8_t block = note / 12;
  uint16_t fnum = noteFNumbers[note % 12];

  // multiplier is only 4bits
  uint8_t freqMultOp1 = (op1Multiplier_.GetInt() & 0xF);
  uint8_t freqMultOp2 = (op2Multiplier_.GetInt() & 0xF);

  uint8_t tremVibSusKSR1 = op1TremVibSusKSR_.GetInt();
  uint8_t tremVibSusKSR2 = op2TremVibSusKSR_.GetInt();

  uint8_t tvskmOp1 = (tremVibSusKSR1 << 4) + freqMultOp1;
  uint8_t tvskmOp2 = (tremVibSusKSR2 << 4) + freqMultOp2;

  // For proper monophonic behavior and to support slides:
  // 1. First update the frequency registers without changing key-on bit
  // 2. Only retrigger the note (key-off then key-on) if retrigger is true

  // Set the frequency (low 8 bits)
  uint8_t areg = fnum & 0xFF;

  // Prepare the block/high-freq bits with key-on bit
  uint8_t new_breg = 0x20 | (block << 2) | (fnum >> 8);

  if (retrigger) {
    // For retriggering, we need to key-off first to restart the envelope
    uint8_t key_off = BitClr(new_breg, 5); // Clear key-on bit from new value
  }

  // Store the new register value for future reference
  breg = new_breg;

  // Note on, block, hi freq - this will set the key-on bit

  // Tremolo/Vibrato/Sustain/KSR/Multiplication

  // 0 = pure sine
  uint8_t waveform1 = op1WaveShape_.GetInt();
  uint8_t waveform2 = op2WaveShape_.GetInt();

  uint8_t keyscale = (op1KeyScaleLevel_.GetInt() & 0x03);
  // level is bottom 6 bits, keyscale top 2 bits
  uint8_t keyscaleOutLvl1 = (keyscale << 6) + (op1Level_.GetInt() & 0x3F);
  uint8_t keyscaleOutLvl2 = (keyscale << 6) + (op2Level_.GetInt() & 0x3F);

  uint16_t adsr1 = op1ADSR_.GetInt();
  uint16_t adsr2 = op2ADSR_.GetInt();

  // Waveform

  // Key Scale Level/Output Level

  // Attack Rate/Decay Rate

  // Sustain Level/Release Rate

  return true;
};

void GameBoyInstrument::Stop(int c) {
};

bool GameBoyInstrument::Render(int channel, fixed *buffer, int size,
                            bool updateTick) {
  PROFILE_SCOPE("GameBoyInstrument::Render");

  // optimise to remove function calls in hot loop
  // opl_.SampleBuffer(buffer, size);

  return true;
};

bool GameBoyInstrument::IsInitialized() {
  return true; // Always initialised
};

void GameBoyInstrument::ProcessCommand(int channel, FourCC cc, ushort value) {
  switch (cc) {
  case FourCC::InstrumentCommandGateOff:
    break;
  }
};

int GameBoyInstrument::GetTable() {
  //  Variable *v = FindVariable(MIP_TABLE);
  //  return v->GetInt();
  return 0;
};

bool GameBoyInstrument::GetTableAutomation() {
  //  Variable *v = FindVariable(MIP_TABLEAUTO);
  //  return v->GetBool();
  return 0;
};

void GameBoyInstrument::GetTableState(TableSaveState &state){

};

void GameBoyInstrument::SetTableState(TableSaveState &state){};
