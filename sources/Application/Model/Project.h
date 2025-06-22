/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _PROJECT_H_
#define _PROJECT_H_

#include "Application/Instruments/InstrumentBank.h"
#include "Application/Persistency/PersistencyService.h"
#include "Application/Persistency/Persistent.h"
#include "BuildNumber.h"
#include "Foundation/Observable.h"
#include "Foundation/Types/Types.h"
#include "Foundation/Variables/VariableContainer.h"
#include "Song.h"

#define PROJECT_NUMBER "2.1-BETA1"
#define PROJECT_RELEASE "r"
// BUILD_COUNT define comes from BuildNumber.h

#define MAX_TAP 3

const unsigned short MAX_TEMPO = 400;
const unsigned short MIN_TEMPO = 60;
const unsigned short DEFAULT_TEMPO = 138;

class Project : public Persistent, public VariableContainer, I_Observer {
public:
  Project(const char *name);
  ~Project();
  void Purge();
  void PurgeInstruments(bool removeFromDisk);

  Song song_;

  int GetMasterVolume();
  int GetChannelVolume(int channel);
  bool Wrap();
  void OnTempoTap();
  void NudgeTempo(int value);
  int GetScale();
  uint8_t GetScaleRoot();
  int GetTempo(); // Takes nudging into account
  int GetTranspose();
  void GetProjectName(char *name);
  void SetProjectName(char *name);

  void Trigger();

  // I_Observer
  virtual void Update(Observable &o, I_ObservableData *d);

  InstrumentBank *GetInstrumentBank();

  // Persistent
  virtual void SaveContent(tinyxml2::XMLPrinter *printer);
  virtual void RestoreContent(PersistencyDocument *doc);

private:
  etl::list<Variable *, 16> variables_;

  InstrumentBank *instrumentBank_;
  int tempoNudge_;
  unsigned long lastTap_[MAX_TAP];
  unsigned int tempoTapCount_;
  char *name[MAX_PROJECT_NAME_LENGTH];

  // variables
  WatchedVariable tempo_;
  Variable masterVolume_;
  // Individual channel volume variables instead of using an array
  // as initialization of such a large array causes in constructors causes stack
  // overflow issues
  Variable channelVolume1_;
  Variable channelVolume2_;
  Variable channelVolume3_;
  Variable channelVolume4_;
  Variable channelVolume5_;
  Variable channelVolume6_;
  Variable channelVolume7_;
  Variable channelVolume8_;
  Variable wrap_;
  Variable transpose_;
  Variable scale_;
  Variable scaleRoot_;
  WatchedVariable projectName_;
  Variable previewVolume_;
};

#endif
