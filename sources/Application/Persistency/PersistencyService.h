#ifndef _PERSISTENCY_SERVICE_H_
#define _PERSISTENCY_SERVICE_H_

#include "Externals/TinyXML2/tinyxml2.h"
#include "Externals/yxml/yxml.h"
#include "Foundation/Services/Service.h"
#include "Foundation/T_Singleton.h"

#define MAX_PROJECT_NAME_LENGTH 16

#define PROJECTS_DIR "/projects"
#define PROJECT_SAMPLES_DIR "samples"
#define SAMPLES_LIB_DIR "/samples"
#define INSTRUMENTS_DIR "/instruments"
#define RENDERS_DIR "/renders"

enum PersistencyResult {
  PERSIST_SAVED,
  PERSIST_LOAD_FAILED,
  PERSIST_LOADED,
};

#define UNNAMED_PROJECT_NAME ".untitled"
#define PROJECT_DATA_FILE "lgptsav.dat"

class PersistencyService : public Service,
                           public T_Singleton<PersistencyService> {
public:
  PersistencyService();
  PersistencyResult Save(const char *projectName, bool saveAs);
  PersistencyResult Load(const char *projectName);
  PersistencyResult LoadCurrentProjectName(char *projectName);
  PersistencyResult SaveProjectState(const char *projectName);
  PersistencyResult CreateProject();
  bool Exists(const char *projectName);
  void PurgeUnnamedProject();
};

#endif
