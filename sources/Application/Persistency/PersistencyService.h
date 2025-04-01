#ifndef _PERSISTENCY_SERVICE_H_
#define _PERSISTENCY_SERVICE_H_

#include "Application/Instruments/I_Instrument.h"
#include "Externals/TinyXML2/tinyxml2.h"
#include "Externals/etl/include/etl/string.h"
#include "Externals/yxml/yxml.h"
#include "Foundation/Services/Service.h"
#include "Foundation/T_Singleton.h"

#define MAX_PROJECT_NAME_LENGTH 16
#define MAX_INSTRUMENT_FILENAME_LENGTH 24
// accounts for .pti extension
#define MAX_INSTRUMENT_NAME_LENGTH (MAX_INSTRUMENT_FILENAME_LENGTH - 4)

#define PROJECTS_DIR "/projects"
#define PROJECT_SAMPLES_DIR "samples"
#define SAMPLES_LIB_DIR "/samples"
#define INSTRUMENTS_DIR "/instruments"
#define RENDERS_DIR "/renders"
#define INSTRUMENT_FILE_EXTENSION ".pti"

enum PersistencyResult {
  PERSIST_SAVED,
  PERSIST_LOAD_FAILED,
  PERSIST_LOADED,
  PERSIST_ERROR,
};

#define UNNAMED_PROJECT_NAME ".untitled"
#define PROJECT_DATA_FILE "lgptsav.dat"
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
  PersistencyResult AutoSaveProjectData(const char *projectName);
  bool ClearAutosave(const char *projectName);

  PersistencyResult
  ExportInstrument(I_Instrument *instrument,
                   etl::string<MAX_INSTRUMENT_NAME_LENGTH> name);
  PersistencyResult ImportInstrument(I_Instrument *instrument,
                                     const char *name);

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
