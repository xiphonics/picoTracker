#include "PersistencyService.h"
#include "../Instruments/SamplePool.h"
#include "Foundation/Types/Types.h"
#include "Persistent.h"
#include "System/Console/Trace.h"

#define PROJECT_STATE_FILE "/.current"

PersistencyService::PersistencyService()
    : Service(FourCC::ServicePersistency){};

PersistencyResult PersistencyService::CreateProject() {
  Trace::Log("APPLICATION", "create new project\n");
  // create  project
  CreateProjectDirs_(UNNAMED_PROJECT_NAME);
  return PersistencyService::GetInstance()->Save(UNNAMED_PROJECT_NAME, "",
                                                 false);
};

void PersistencyService::PurgeUnnamedProject() {
  auto picoFS = PicoFileSystem::GetInstance();

  picoFS->chdir(PROJECTS_DIR);
  Trace::Debug("PERSISTENCYSERVICE", "purging unnamed project dir\n");
  picoFS->chdir(UNNAMED_PROJECT_NAME);
  picoFS->DeleteFile(PROJECT_DATA_FILE);

  picoFS->chdir("samples");
  etl::vector<int, MAX_PIG_SAMPLES> fileIndexes;
  picoFS->list(&fileIndexes, ".wav", false);

  // delete all samples
  char filename[128];
  for (size_t i = 0; i < fileIndexes.size(); i++) {
    picoFS->getFileName(fileIndexes[i], filename,
                        MAX_PROJECT_SAMPLE_PATH_LENGTH);
    picoFS->DeleteFile(filename);
  };
};

PersistencyResult
PersistencyService::CreateProjectDirs_(const char *projectName) {
  auto picoFS = PicoFileSystem::GetInstance();

  etl::vector segments{PROJECTS_DIR, projectName};
  CreatePath(pathBufferA, segments);

  bool result = picoFS->makeDir(pathBufferA.c_str());
  Trace::Log("PERSISTENCYSERVICE", "created project dir: %s\n [%b]",
             pathBufferA.c_str(), result);
  if (!result) {
    return PersistencyResult::PERSIST_ERROR;
  }

  // also create samples sub dir
  segments = {PROJECT_SAMPLES_DIR};
  CreatePath(pathBufferA, segments);

  result = picoFS->makeDir(pathBufferA.c_str());
  Trace::Log("PERSISTENCYSERVICE", "created samples subdir: %s\n [%b]",
             pathBufferA.c_str(), result);

  return result ? PersistencyResult::PERSIST_SAVED
                : PersistencyResult::PERSIST_ERROR;
}

PersistencyResult PersistencyService::Save(const char *projectName,
                                           const char *oldProjectName,
                                           bool saveAs) {
  auto picoFS = PicoFileSystem::GetInstance();

  if (saveAs && !Exists(projectName)) {
    CreateProjectDirs_(projectName);

    // copy across the samples from the old project
    picoFS->chdir(oldProjectName);
    picoFS->chdir(PROJECT_SAMPLES_DIR);

    Trace::Debug("list samples to copyfrom old project: %s\n", oldProjectName);

    picoFS->list(&fileIndexes_, ".wav", false);
    char filenameBuffer[32];
    for (size_t i = 0; i < fileIndexes_.size(); i++) {
      picoFS->getFileName(fileIndexes_[i], filenameBuffer,
                          MAX_PROJECT_SAMPLE_PATH_LENGTH);

      // ignore . and .. entries as using *.wav doesnt filter them out
      if (strcmp(filenameBuffer, ".") == 0 || strcmp(filenameBuffer, "..") == 0)
        continue;

      etl::vector filePathSegments = {PROJECTS_DIR, oldProjectName,
                                      PROJECT_SAMPLES_DIR, filenameBuffer};
      CreatePath(pathBufferA, filePathSegments);

      filePathSegments = {PROJECTS_DIR, projectName, PROJECT_SAMPLES_DIR,
                          filenameBuffer};
      CreatePath(pathBufferB, filePathSegments);

      picoFS->CopyFile(pathBufferA.c_str(), pathBufferB.c_str());
    };
  }

  etl::vector segments = {PROJECTS_DIR, projectName, PROJECT_DATA_FILE};
  CreatePath(pathBufferA, segments);

  PI_File *fp = picoFS->Open(pathBufferA.c_str(), "w");
  if (!fp) {
    Trace::Error("PERSISTENCYSERVICE: Could not open file for writing: %s",
                 pathBufferA.c_str());
  }
  Trace::Log("PERSISTENCYSERVICE", "Opened Proj File: %s\n",
             pathBufferA.c_str());
  tinyxml2::XMLPrinter printer(fp);
  Trace::Log("PERSISTENCYSERVICE", "Saved Proj File: %s\n",
             pathBufferA.c_str());

  printer.OpenElement("PICOTRACKER");

  // Loop on all registered service
  // accumulating XML flow
  for (Begin(); !IsDone(); Next()) {
    Persistent *currentItem = (Persistent *)&CurrentItem();
    currentItem->Save(&printer);
  };

  printer.CloseElement();

  fp->Close();
  delete (fp);
  return PERSIST_SAVED;
};

// return true if existing proj with the given name already exists
bool PersistencyService::Exists(const char *projectName) {
  etl::string<128> projectFilePath(PROJECTS_DIR);
  projectFilePath.append("/");
  projectFilePath.append(projectName);

  auto picoFS = PicoFileSystem::GetInstance();
  return picoFS->exists(projectFilePath.c_str());
}

PersistencyResult PersistencyService::Load(const char *projectName) {
  etl::string<128> projectFilePath(PROJECTS_DIR);
  projectFilePath.append("/");
  projectFilePath.append(projectName);
  projectFilePath.append("/");
  projectFilePath.append(PROJECT_DATA_FILE);

  PersistencyDocument doc;
  if (!doc.Load(projectFilePath.c_str()))
    return PERSIST_LOAD_FAILED;

  bool elem = doc.FirstChild(); // advance to first child
  if (!elem || strcmp(doc.ElemName(), "PICOTRACKER")) {
    Trace::Error("could not find master node");
    return PERSIST_LOAD_FAILED;
  }

  elem = doc.FirstChild();
  while (elem) {
    for (Begin(); !IsDone(); Next()) {
      Persistent *currentItem = (Persistent *)&CurrentItem();
      if (currentItem->Restore(&doc)) {
        break;
      };
    }
    elem = doc.NextSibling();
  }
  return PERSIST_LOADED;
};

PersistencyResult
PersistencyService::LoadCurrentProjectName(char *projectName) {
  auto picoFS = PicoFileSystem::GetInstance();
  if (picoFS->exists(PROJECT_STATE_FILE)) {
    auto current = picoFS->Open(PROJECT_STATE_FILE, "r");
    int len = current->Read(projectName, MAX_PROJECT_NAME_LENGTH - 1);
    current->Close();
    projectName[len] = '\0';
    Trace::Log("APPLICATION", "read [%d] load proj name: %s\n", len,
               projectName);
    return PERSIST_LOADED;
  } else {
    return PERSIST_LOAD_FAILED;
  }
}

PersistencyResult
PersistencyService::SaveProjectState(const char *projectName) {
  auto picoFS = PicoFileSystem::GetInstance();
  auto current = picoFS->Open(PROJECT_STATE_FILE, "w");
  current->Write(projectName, 1, strlen(projectName));
  current->Close();
  return PERSIST_SAVED;
}

void PersistencyService::CreatePath(
    etl::istring &path, const etl::ivector<const char *> &segments) {
  // concatenate path segments into a single path
  path.clear();
  // iterate over segments and concatenate using iterator
  for (auto it = segments.begin(); it != segments.end(); ++it) {
    path.append("/");
    path.append(*it);
  }
}
