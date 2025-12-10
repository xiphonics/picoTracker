/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _PERSISTENCY_SERVICE_H_
#define _PERSISTENCY_SERVICE_H_

#include "Application/Instruments/I_Instrument.h"
#include "Externals/TinyXML2/tinyxml2.h"
#include "Externals/etl/include/etl/string.h"
#include "Externals/yxml/yxml.h"

#include "Foundation/Services/Service.h"
#include "Foundation/T_Singleton.h"
#include "PersistenceConstants.h"

enum PersistencyResult {
  PERSIST_SAVED,
  PERSIST_LOAD_FAILED,
  PERSIST_LOADED,
  PERSIST_ERROR,
  PERSIST_EXISTS,
};

#define UNNAMED_PROJECT_NAME ".untitled"
#ifdef ADV
#define PROJECT_DATA_FILE "ptsav.dat"
#else
#define PROJECT_DATA_FILE "lgptsav.dat"
#endif
#define AUTO_SAVE_FILENAME "autosave.dat"

class PersistencyService : public Service,
                           public T_Singleton<PersistencyService> {
public:
  PersistencyService();
  PersistencyResult Save(const char *projectName, const char *oldProjectName,
                         bool saveAs);
  PersistencyResult Load(const char *projectName);
  PersistencyResult LoadCurrentProjectName(char *projectName);
  PersistencyResult SaveProjectState(const char *projectName);
  PersistencyResult CreateProject();
  bool Exists(const char *projectName);
  void PurgeUnnamedProject();
  void DeleteProject(const char *projectName);
  PersistencyResult AutoSaveProjectData(const char *projectName);
  bool ClearAutosave(const char *projectName);

  PersistencyResult
  ExportInstrument(I_Instrument *instrument,
                   etl::string<MAX_INSTRUMENT_NAME_LENGTH> name,
                   bool overwrite = false);
  PersistencyResult ImportInstrument(I_Instrument *instrument,
                                     const char *name);
  InstrumentType DetectInstrumentType(const char *name);

private:
  PersistencyResult CreateProjectDirs_(const char *projectName);
  void CreatePath(etl::istring &path,
                  const etl::ivector<const char *> &segments);
  PersistencyResult SaveProjectData(const char *projectName, bool autosave);

  // need these as statically allocated buffers as too big for stack
  etl::vector<int, MAX_FILE_INDEX_SIZE> fileIndexes_;
  etl::string<MAX_PROJECT_SAMPLE_PATH_LENGTH> pathBufferA;
  etl::string<MAX_PROJECT_SAMPLE_PATH_LENGTH> pathBufferB;
};

#endif
