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

#define PROJECT_NUMBER "1.99"
#define PROJECT_RELEASE "r"
// BUILD_COUNT define comes from BuildNumber.h

#define MAX_TAP 3

class Project : public Persistent, public VariableContainer, I_Observer {
public:
  Project(const char *name);
  ~Project();
  void Purge();
  void PurgeInstruments(bool removeFromDisk);

  Song song_;

  int GetMasterVolume();
  bool Wrap();
  void OnTempoTap();
  void NudgeTempo(int value);
  int GetScale();
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
  InstrumentBank *instrumentBank_;
  int tempoNudge_;
  unsigned long lastTap_[MAX_TAP];
  unsigned int tempoTapCount_;
  char *name[MAX_PROJECT_NAME_LENGTH];

  // variables
  WatchedVariable tempo_;
  Variable masterVolume_;
  Variable wrap_;
  Variable transpose_;
  Variable scale_;
  Variable projectName_;
};

#endif
