/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _SAMPLE_INSTRUMENT_H_
#define _SAMPLE_INSTRUMENT_H_

#include "Application/Model/Song.h"
#include "Application/Persistency/PersistenceConstants.h"
#include "Foundation/Observable.h"
#include "Foundation/Types/Types.h"
#include "Foundation/Variables/WatchedVariable.h"
#include "I_Instrument.h"
#include "SRPUpdaters.h"
#include "SampleRenderingParams.h"
#include "SampleVariable.h"
#include "SoundSource.h"

enum SampleInstrumentLoopMode {
  SILM_ONESHOT = 0,
  SILM_LOOP,
  SILM_LOOP_PINGPONG,
  SILM_OSC,
  //	SILM_OSCFINE,
  SILM_LOOPSYNC,
  SILM_LAST
};

#define NO_SAMPLE (-1)

class SampleInstrument : public I_Instrument, I_Observer {

public:
  SampleInstrument();
  virtual ~SampleInstrument();
  // I_Instrument implementation
  virtual bool Init();
  virtual bool Start(int channel, unsigned char note, bool trigger = true);
  virtual void Stop(int channel);
  virtual bool Render(int channel, fixed *buffer, int size, bool updateTick);
  virtual bool IsInitialized();
  virtual bool IsEmpty();

  virtual InstrumentType GetType() { return IT_SAMPLE; };
  virtual void ProcessCommand(int channel, FourCC cc, ushort value);
  virtual void Purge();
  virtual int GetTable();
  virtual bool GetTableAutomation();
  virtual void GetTableState(TableSaveState &state);
  virtual void SetTableState(TableSaveState &state);
  etl::ilist<Variable *> *Variables() { return &variables_; };

  bool IsMulti();

  // Engine playback  start callback

  virtual void OnStart();

  // I_Observer
  virtual void Update(Observable &o, I_ObservableData *d);
  // Additional
  void AssignSample(int i);
  int GetSampleIndex();
  int GetVolume();
  void SetVolume(int);
  int GetSampleSize(int channel = -1);
  float GetLengthInSec();

  virtual etl::string<MAX_INSTRUMENT_NAME_LENGTH> GetUserSetName();
  virtual etl::string<MAX_INSTRUMENT_NAME_LENGTH> GetDisplayName() override;
  virtual etl::string<MAX_INSTRUMENT_NAME_LENGTH> GetSampleFileName();

  static void EnableDownsamplingLegacy();

protected:
  void updateInstrumentData(bool search);
  void doTickUpdate(int channel);
  void doKRateUpdate(int channel);

private:
  etl::list<Variable *, 21> variables_;

  SoundSource *source_;
  __attribute__((section(".DTCMRAM"))) static struct renderParams
      renderParams_[SONG_CHANNEL_COUNT];
  bool running_;
  bool dirty_;
  TableSaveState tableState_;

  static signed char lastMidiNote_[SONG_CHANNEL_COUNT];
  static fixed lastSample_[SONG_CHANNEL_COUNT][2];
  SampleVariable sample_;
  Variable volume_;
  Variable interpolation_;
  Variable crush_;
  Variable drive_;
  Variable downsample_;
  Variable rootNote_;
  Variable fineTune_;
  Variable pan_;
  Variable cutoff_;
  Variable reso_;
  Variable filterMix_;
  Variable filterMode_;
  WatchedVariable start_;
  Variable loopMode_;
  WatchedVariable loopStart_;
  WatchedVariable loopEnd_;
  Variable table_;
  Variable tableAuto_;

  static bool useDirtyDownsampling_;
};
#endif
