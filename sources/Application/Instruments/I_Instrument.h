/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _I_INSTRUMENT_H_
#define _I_INSTRUMENT_H_

#include "Application/Persistency/PersistenceConstants.h"
#include "Application/Persistency/Persistent.h"
#include "Application/Player/TablePlayback.h"
#include "Application/Utils/fixed.h"
#include "Application/Utils/stringutils.h"
#include "Externals/etl/include/etl/string.h"
#include "Foundation/Observable.h"
#include "Foundation/Variables/VariableContainer.h"

enum InstrumentType {
  IT_NONE = 0,
  IT_SAMPLE,
  IT_MIDI,
  IT_SID,
  IT_OPAL,
  IT_LAST
};
static const char *InstrumentTypeNames[IT_LAST] = {"NONE", "SAMPLE", "MIDI",
                                                   "SID", "OPAL"};

class I_Instrument : public VariableContainer,
                     public Observable,
                     public Persistent {
protected:
  etl::string<MAX_INSTRUMENT_NAME_LENGTH> name_;

public:
  I_Instrument(etl::ilist<Variable *> *list,
               const char *nodeName = "INSTRUMENT")
      : VariableContainer(list), Persistent(nodeName){};
  virtual ~I_Instrument();

  // Initialisation routine

  virtual bool Init() = 0;

  // Start & stop the instument
  virtual bool Start(int channel, unsigned char note,
                     bool retrigger = true) = 0;
  virtual void Stop(int channel) = 0;

  // Engine playback  start callback

  virtual void OnStart() = 0;

  // size refers to the number of samples
  // should always fill interleaved stereo / 16bit
  // return value is true if any audio was rendered
  virtual bool Render(int channel, fixed *buffer, int size,
                      bool updateTick) = 0;

  virtual bool IsInitialized() = 0;

  virtual bool IsEmpty() = 0;

  virtual InstrumentType GetType() = 0;

  virtual etl::string<MAX_INSTRUMENT_NAME_LENGTH> GetDefaultName() {
    return etl::string<MAX_INSTRUMENT_NAME_LENGTH>(
        InstrumentTypeNames[GetType()]);
  };

  virtual etl::string<MAX_INSTRUMENT_NAME_LENGTH> GetUserSetName() {
    return name_;
  };

  // Set the instrument name
  virtual void SetName(const char *name) {
    name_ = name;
    SetChanged();
    NotifyObservers();
  };

  // Set the instrument name from a Variable
  virtual void SetNameFromVariable(Variable *nameVar) {
    if (nameVar) {
      name_ = nameVar->GetString();
      SetChanged();
      NotifyObservers();
    }
  };

  // return the name to display in the UI
  // will be the user set name if available other the default name is returned
  virtual etl::string<MAX_INSTRUMENT_NAME_LENGTH> GetDisplayName() {
    auto name = GetUserSetName();
    if (!name.empty()) {
      return name;
    }
    return GetDefaultName();
  }

  virtual void ProcessCommand(int channel, FourCC cc, ushort value) = 0;

  virtual void Purge() = 0;

  virtual int GetTable() = 0;
  virtual bool GetTableAutomation() = 0;

  virtual void GetTableState(TableSaveState &state) = 0;
  virtual void SetTableState(TableSaveState &state) = 0;
  virtual etl::ilist<Variable *> *Variables() = 0;

  // Persistent implementation
  virtual void SaveContent(tinyxml2::XMLPrinter *printer) override;
  virtual void RestoreContent(PersistencyDocument *doc) override;
};
#endif
