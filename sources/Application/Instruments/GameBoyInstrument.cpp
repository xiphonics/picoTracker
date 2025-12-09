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

static const char *waveShapes[GB_NUM_WAVEFORMS] = {"Pulse 12.5%", "Pulse 25%", 
                                                   "Pulse 50%", "Triangle", 
                                                   "Noise"};

#define CHANNEL 0 // just hardcoding to channel 0 for now

static const uint32_t frequencyTable[128 + 24] = {
398127	,421801	,446882	,473455	,501608	,531436	,563036	,596516	,631987	,669567	,709381	,751563	,
796254	,843601	,893765	,946911	,1003217	,1062871	,1126073	,1193033	,1263974	,1339134	,1418763	,1503127	,
1592507	,1687203	,1787529	,1893821	,2006434	,2125742	,2252146	,2386065	,2527948	,2678268	,2837526	,3006254	,
3185015	,3374406	,3575058	,3787642	,4012867	,4251485	,4504291	,4772130	,5055896	,5356535	,5675051	,6012507	,
6370030	,6748811	,7150117	,7575285	,8025735	,8502970	,9008582	,9544261	,10111792	,10713070	,11350103	,12025015	,
12740059	,13497623	,14300233	,15150569	,16051469	,17005939	,18017165	,19088521	,20223584	,21426141	,22700205	,24050030	,
25480119	,26995246	,28600467	,30301139	,32102938	,34011878	,36034330	,38177043	,40447168	,42852281	,45400411	,48100060	,
50960238	,53990491	,57200933	,60602278	,64205876	,68023757	,72068660	,76354085	,80894335	,85704563	,90800821	,96200119	,
101920476	,107980983	,114401866	,121204555	,128411753	,136047513	,144137319	,152708170	,161788671	,171409126	,181601643	,192400238	,
203840952	,215961966	,228803732	,242409110	,256823506	,272095026	,288274639	,305416341	,323577341	,342818251	,363203285	,384800477	,
407681904	,431923931	,457607465	,484818220	,513647012	,544190053	,576549277	,610832681	,647154683	,685636503	,726406571	,769600953	,
815363807	,863847862	,915214929	,969636441	,1027294024	,1088380105	,1153098554	,1221665363	,1294309365	,1371273005	,1452813141	,1539201906	,
1630727614	,1727695724	,1830429858	,1939272882	,2054588048	,2176760211	,2306197109	,2443330725	
};

GameBoyInstrument::GameBoyInstrument()
    : I_Instrument(&variables_),
      waveform_(FourCC::GameBoyInstrumentWaveform, waveShapes, 5, 0),
      attack_(FourCC::GameBoyInstrumentAttack, 0x0FF),
      decay_(FourCC::GameBoyInstrumentDecay, 0x0FF),
      level_(FourCC::GameBoyInstrumentLevel, 0x80),
      length_(FourCC::GameBoyInstrumentLength, 0x40),
      burst_(FourCC::GameBoyInstrumentBurst, 0x00),
      vibrato_(FourCC::GameBoyInstrumentVibrato, 0x07),
      vibratoDelay_(FourCC::GameBoyInstrumentVibratoDelay, 0x40),
      transpose_(FourCC::GameBoyInstrumentTranspose, 0x00),
      table_(FourCC::GameBoyInstrumentTable, 0x00),
      sweepTime_(FourCC::GameBoyInstrumentSweepTime, 0x00),
      sweepAmount_(FourCC::GameBoyInstrumentSweepAmount, 0x00) {

  // name_ is now an etl::string in the base class, not a Variable
  variables_.insert(variables_.end(), &waveform_);
  variables_.insert(variables_.end(), &attack_);
  variables_.insert(variables_.end(), &decay_);
  variables_.insert(variables_.end(), &level_);
  variables_.insert(variables_.end(), &length_);
  variables_.insert(variables_.end(), &burst_);
  variables_.insert(variables_.end(), &vibrato_);
  variables_.insert(variables_.end(), &vibratoDelay_);
  variables_.insert(variables_.end(), &transpose_);
  variables_.insert(variables_.end(), &table_);
  variables_.insert(variables_.end(), &sweepTime_);
  variables_.insert(variables_.end(), &sweepAmount_);
}

GameBoyInstrument::~GameBoyInstrument(){};

bool GameBoyInstrument::Init() {
  // enable left/right only for 0 channel

  return true;
};

void GameBoyInstrument::OnStart(){};

bool GameBoyInstrument::Start(int channel, unsigned char note, bool retrigger) {
  // note on get frequency etc.
  
  // int fIndex = getStepForMidi(note + transpose_.GetInt());
  frequency_ = frequencyTable[note + 12];
  Trace::Error("note %d freq %d", note, frequency_);
  
  phase_ = 0;

  return true;
};

void GameBoyInstrument::Stop(int c) {
  frequency_ = 0;
  phase_ = 0;
};

typedef struct {
    bool prevState;     // +1 or -1
    int   step;          // 0..KERNEL_LEN
    int   active;        // 0 = no BLEP, +1 rising, -1 falling
} PulseState;

PulseState st = { false, 0, 0 };

#define KERNEL_LEN 5
static const uint32_t kernel5[KERNEL_LEN] = {
  0x01400004,
  0x047FFFF7,
  0x08CF1FBC,
  0x0D204B51,
  0x105F96B4
};
static const uint32_t invkernel5[KERNEL_LEN] = {
    0x0fff'ffff - kernel5[0],
    0x0fff'ffff - kernel5[1],
    0x0fff'ffff - kernel5[2],
    0x0fff'ffff - kernel5[3],
    0x0fff'ffff - kernel5[4]
};

// kernel5 is assumed to be float kernel5[KERNEL_LEN]

static inline uint32_t pulse(bool high)
{
    // Detect transition
    if (high != st.prevState) {
      st.active = high ? +1 : -1; // +1 = rising, -1 = falling
      st.step = 0;
      st.prevState = high;
    }

    // BLEP in progress?
    if (st.active == 1) {
        if (st.step >= KERNEL_LEN) st.active = 0;
        return kernel5[st.step++];
    }
    if (st.active == -1) {
      if (st.step >= KERNEL_LEN) st.active = 0;
      return invkernel5[st.step++];
    }

    // Stable
    return high ? 0x0fff'ffff : 0x0000'0000;
}

bool GameBoyInstrument::Render(int channel, fixed *buffer, int size,
                            bool updateTick) {
  // PROFILE_SCOPE("GameBoyInstrument::Render");

  const int wave = waveform_.GetInt();

  uint16_t level = level_.GetInt();

  static uint32_t lastSample = 0;                        

  for (int s = 0; s < size; s++) {
    // advance phase
    phase_ += frequency_;
    fixed sample = phase_ >> 16;

    // generate sample based on waveform
    switch (wave) {
      case 0: // Pulse 12.5%
        sample = pulse(phase_ > 0x2000'0000);
        break;
      case 1: // Pulse 25%
        sample = pulse(phase_ > 0x4000'0000);
        break;
      case 2: // Pulse 50%
        sample = pulse(phase_ > 0x8000'0000);
        break;
      case 3: // Triangle
        sample = (phase_ < 0x8000'0000) ? phase_ >> 3 : (0xffffffff - (phase_ >> 3));
        sample &= 0xf00000;
        break;
      case 4: // Noise
        sample = rand() & 0xfffffff;
        break;
    }

    sample = (sample >> 1) + (lastSample >> 1);
    lastSample = sample;

    // output to both left and right channels
    buffer[s * 2] = sample;
    buffer[s * 2 + 1] = sample;
  }

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

}

void GameBoyInstrument::SetTableState(TableSaveState &state){

}
