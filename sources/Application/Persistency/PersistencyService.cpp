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
  picoFS->DeleteFile(AUTO_SAVE_FILENAME);

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

  // create samples sub dir as well as project dir containing it
  etl::vector segments = {PROJECTS_DIR, projectName, PROJECT_SAMPLES_DIR};
  CreatePath(pathBufferA, segments);

  auto result = picoFS->makeDir(pathBufferA.c_str(), true);
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
    picoFS->chdir(PROJECTS_DIR);
    picoFS->chdir(oldProjectName);
    picoFS->chdir(PROJECT_SAMPLES_DIR);

    Trace::Debug("get list of samples to copy from old project: %s\n",
                 oldProjectName);

    picoFS->list(&fileIndexes_, ".wav", false);
    char filenameBuffer[PFILENAME_SIZE];
    for (size_t i = 0; i < fileIndexes_.size(); i++) {
      picoFS->getFileName(fileIndexes_[i], filenameBuffer,
                          sizeof(filenameBuffer));

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
  return SaveProjectData(projectName, false);
};

PersistencyResult
PersistencyService::AutoSaveProjectData(const char *projectName) {
  return SaveProjectData(projectName, true);
};

PersistencyResult PersistencyService::SaveProjectData(const char *projectName,
                                                      bool autosave) {

  const char *filename = autosave ? AUTO_SAVE_FILENAME : PROJECT_DATA_FILE;

  etl::vector segments = {PROJECTS_DIR, projectName, filename};
  CreatePath(pathBufferA, segments);

  auto picoFS = PicoFileSystem::GetInstance();
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

  // if we are doing an explicit save (ie nto a autosave), then we need to
  // delete the existing autosave file so that this explicit save will be loaded
  // in case subsequent autosave has changes the user doesn't want to keep
  if (!autosave) {
    if (!ClearAutosave(projectName)) {
      Trace::Log("PERSISTENCYSERVICE", "Error Deleting Autosave File: %s\n",
                 pathBufferA.c_str());
      // the autosave file may not have been created yet, eg. if this is a new
      // project or a project beign "saved as" so just keep going
    }
    Trace::Log("PERSISTENCYSERVICE", "Deleted Autosave File: %s\n",
               pathBufferA.c_str());
  }

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
  // first check if autosave exists
  etl::string<128> autoSavePath(PROJECTS_DIR);
  autoSavePath.append("/");
  autoSavePath.append(projectName);
  autoSavePath.append("/");
  autoSavePath.append(AUTO_SAVE_FILENAME);

  auto picoFS = PicoFileSystem::GetInstance();
  bool useAutosave = (picoFS->exists(autoSavePath.c_str()));

  Trace::Log("PERSISTENCYSERVICE", "using autosave: %b\n", useAutosave);
  // if autosave exists, then we load it instead of the normal project file
  const char *filename = useAutosave ? AUTO_SAVE_FILENAME : PROJECT_DATA_FILE;

  etl::string<128> projectFilePath(PROJECTS_DIR);
  projectFilePath.append("/");
  projectFilePath.append(projectName);
  projectFilePath.append("/");
  projectFilePath.append(filename);

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
    path.append(*it);
    if (it != segments.end() - 1) {
      path.append("/");
    }
  }
}

bool PersistencyService::ClearAutosave(const char *projectName) {
  auto picoFS = PicoFileSystem::GetInstance();
  etl::vector segments = {PROJECTS_DIR, projectName, AUTO_SAVE_FILENAME};
  CreatePath(pathBufferA, segments);
  return picoFS->DeleteFile(pathBufferA.c_str());
}