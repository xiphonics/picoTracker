#include "OpalInstrument.h"
#include "CommandList.h"
#include "Externals/etl/include/etl/to_string.h"
#include "I_Instrument.h"
#include "System/Console/Trace.h"
#include "System/Performance/Profiler.h"
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

OpalInstrument::OpalInstrument()
    : I_Instrument(&variables_),
      algorithm_(FourCC::OPALInstrumentAlgorithm, algorithms, 6, 0),
      feedback_(FourCC::OPALInstrumentFeedback, 0),
      deepTremeloVibrato_(FourCC::OPALInstrumentDeepTremeloVibrato, 0),
      op1Level_(FourCC::OPALInstrumentOp1Level, 0x17),
      op1Multiplier_(FourCC::OPALInstrumentOp1Multiplier, 0x1),
      op1ADSR_(FourCC::OPALInstrumentOp1ADSR, 0xF1C8),
      op1WaveShape_(FourCC::OPALInstrumentOp1WaveShape, waveShapes, 8, 0),
      op1TremVibSusKSR_(FourCC::OPALInstrumentOp1TremVibSusKSR, 0),
      op1KeyScaleLevel_(FourCC::OPALInstrumentOp1KeyScaleLevel, kslValues, 4,
                        0x1),
      op2Level_(FourCC::OPALInstrumentOp2Level, 0),
      op2Multiplier_(FourCC::OPALInstrumentOp2Multiplier, 0x1),
      op2ADSR_(FourCC::OPALInstrumentOp2ADSR, 0xF1D8),
      op2WaveShape_(FourCC::OPALInstrumentOp2WaveShape, waveShapes, 8, 0),
      op2TremVibSusKSR_(FourCC::OPALInstrumentOp2TremVibSusKSR, 0x2),
      op2KeyScaleLevel_(FourCC::OPALInstrumentOp2KeyScaleLevel, kslValues, 4,
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

OpalInstrument::~OpalInstrument(){};

bool OpalInstrument::Init() {
  // enable left/right only for 0 channel
  opl_.Port(0xC0 + CHANNEL, 0x30);

  return true;
};

void OpalInstrument::OnStart(){};

bool OpalInstrument::Start(int channel, unsigned char note, bool retrigger) {
  // channel wide settings
  // enable left/right output (D4, D5) & set algorithm D0
  // for now only 2 op so just Additive or FM
  opl_.Port(0xC0 + CHANNEL, 0x30 + algorithm_.GetInt());

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
  opl_.Port(FREQ_BASE_REG + CHANNEL, areg);

  // Prepare the block/high-freq bits with key-on bit
  uint8_t new_breg = 0x20 | (block << 2) | (fnum >> 8);

  if (retrigger) {
    // For retriggering, we need to key-off first to restart the envelope
    uint8_t key_off = BitClr(new_breg, 5); // Clear key-on bit from new value
    opl_.Port(OCTAVE_BASE_REG + CHANNEL, key_off);
  }

  // Store the new register value for future reference
  breg = new_breg;

  // Note on, block, hi freq - this will set the key-on bit
  opl_.Port(OCTAVE_BASE_REG + CHANNEL, breg);
  // Tremolo/Vibrato/Sustain/KSR/Multiplication
  opl_.Port(0x20 + CHANNEL, tvskmOp1);
  opl_.Port(0x21 + CHANNEL, tvskmOp2);

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
  opl_.Port(0xE0 + CHANNEL, waveform1);
  opl_.Port(0xE1 + CHANNEL, waveform2);

  // Key Scale Level/Output Level
  opl_.Port(0x40 + CHANNEL, keyscaleOutLvl1);
  opl_.Port(0x41 + CHANNEL, keyscaleOutLvl2);

  // Attack Rate/Decay Rate
  opl_.Port(0x60 + CHANNEL, adsr1 >> 8);
  opl_.Port(0x61 + CHANNEL, adsr2 >> 8);

  // Sustain Level/Release Rate
  opl_.Port(0x80 + CHANNEL, (uint8_t)(adsr1 & 0x00FF));
  opl_.Port(0x81 + CHANNEL, (uint8_t)(adsr2 & 0x00FF));

  return true;
};

void OpalInstrument::Stop(int c) {
  uint8_t stop = BitClr(breg, 5);
  opl_.Port(OCTAVE_BASE_REG, stop);
};

bool OpalInstrument::Render(int channel, fixed *buffer, int size,
                            bool updateTick) {
  PROFILE_SCOPE("OpalInstrument::Render");

  // optimise to remove function calls in hot loop
  opl_.SampleBuffer(buffer, size);

  return true;
};

bool OpalInstrument::IsInitialized() {
  return true; // Always initialised
};

void OpalInstrument::ProcessCommand(int channel, FourCC cc, ushort value) {
  switch (cc) {
  case FourCC::InstrumentCommandGateOff:
    uint8_t stop = BitClr(breg, 5);
    opl_.Port(OCTAVE_BASE_REG, stop);
    break;
  }
};

int OpalInstrument::GetTable() {
  //  Variable *v = FindVariable(MIP_TABLE);
  //  return v->GetInt();
  return 0;
};

bool OpalInstrument::GetTableAutomation() {
  //  Variable *v = FindVariable(MIP_TABLEAUTO);
  //  return v->GetBool();
  return 0;
};

void OpalInstrument::GetTableState(TableSaveState &state){

};

void OpalInstrument::SetTableState(TableSaveState &state){};
