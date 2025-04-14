#include "PersistencyService.h"
#include "../Instruments/SamplePool.h"
#include "Foundation/Types/Types.h"
#include "Persistent.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/PI_File.h"

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
  auto fs = FileSystem::GetInstance();

  fs->chdir(PROJECTS_DIR);
  Trace::Debug("PERSISTENCYSERVICE", "purging unnamed project dir\n");
  fs->chdir(UNNAMED_PROJECT_NAME);
  fs->DeleteFile(PROJECT_DATA_FILE);
  fs->DeleteFile(AUTO_SAVE_FILENAME);

  fs->chdir("samples");
  etl::vector<int, MAX_PIG_SAMPLES> fileIndexes;
  fs->list(&fileIndexes, ".wav", false);

  // delete all samples
  char filename[128];
  for (size_t i = 0; i < fileIndexes.size(); i++) {
    fs->getFileName(fileIndexes[i], filename, MAX_PROJECT_SAMPLE_PATH_LENGTH);
    fs->DeleteFile(filename);
  };
};

PersistencyResult
PersistencyService::CreateProjectDirs_(const char *projectName) {
  auto fs = FileSystem::GetInstance();

  // create samples sub dir as well as project dir containing it
  etl::vector<const char *, 3> segments = {PROJECTS_DIR, projectName,
                                           PROJECT_SAMPLES_DIR};
  CreatePath(pathBufferA, segments);

  auto result = fs->makeDir(pathBufferA.c_str(), true);
  Trace::Log("PERSISTENCYSERVICE", "created samples subdir: %s\n [%b]",
             pathBufferA.c_str(), result);

  return result ? PersistencyResult::PERSIST_SAVED
                : PersistencyResult::PERSIST_ERROR;
}

PersistencyResult PersistencyService::Save(const char *projectName,
                                           const char *oldProjectName,
                                           bool saveAs) {
  auto fs = FileSystem::GetInstance();

  if (saveAs && !Exists(projectName)) {
    CreateProjectDirs_(projectName);

    // copy across the samples from the old project
    fs->chdir(PROJECTS_DIR);
    fs->chdir(oldProjectName);
    fs->chdir(PROJECT_SAMPLES_DIR);

    Trace::Debug("get list of samples to copy from old project: %s\n",
                 oldProjectName);

    fs->list(&fileIndexes_, ".wav", false);
    char filenameBuffer[PFILENAME_SIZE];
    for (size_t i = 0; i < fileIndexes_.size(); i++) {
      fs->getFileName(fileIndexes_[i], filenameBuffer, sizeof(filenameBuffer));

      // ignore . and .. entries as using *.wav doesnt filter them out
      if (strcmp(filenameBuffer, ".") == 0 || strcmp(filenameBuffer, "..") == 0)
        continue;

      etl::vector<const char *, 4> filePathSegments = {
          PROJECTS_DIR, oldProjectName, PROJECT_SAMPLES_DIR, filenameBuffer};
      CreatePath(pathBufferA, filePathSegments);

      filePathSegments = {PROJECTS_DIR, projectName, PROJECT_SAMPLES_DIR,
                          filenameBuffer};
      CreatePath(pathBufferB, filePathSegments);

      fs->CopyFile(pathBufferA.c_str(), pathBufferB.c_str());
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

  etl::vector<const char *, 3> segments = {PROJECTS_DIR, projectName, filename};
  CreatePath(pathBufferA, segments);

  auto fs = FileSystem::GetInstance();
  PI_File *fp = fs->Open(pathBufferA.c_str(), "w");
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

  auto fs = FileSystem::GetInstance();
  return fs->exists(projectFilePath.c_str());
}

PersistencyResult PersistencyService::Load(const char *projectName) {
  // first check if autosave exists
  etl::string<128> autoSavePath(PROJECTS_DIR);
  autoSavePath.append("/");
  autoSavePath.append(projectName);
  autoSavePath.append("/");
  autoSavePath.append(AUTO_SAVE_FILENAME);

  auto fs = FileSystem::GetInstance();
  bool useAutosave = (fs->exists(autoSavePath.c_str()));

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
  auto fs = FileSystem::GetInstance();
  if (fs->exists(PROJECT_STATE_FILE)) {
    auto current = fs->Open(PROJECT_STATE_FILE, "r");
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
  auto fs = FileSystem::GetInstance();
  auto current = fs->Open(PROJECT_STATE_FILE, "w");
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
  etl::vector<const char *, 3> segments = {PROJECTS_DIR, projectName,
                                           AUTO_SAVE_FILENAME};
  CreatePath(pathBufferA, segments);
  // TODO: check if file exists before deleting and only return false if it does
  // exist and deleting fails but this can only be done once Open() return
  // values are improved and we can implement a Exists() function on top of it
  return picoFS->DeleteFile(pathBufferA.c_str());
}

PersistencyResult PersistencyService::ExportInstrument(
    I_Instrument *instrument, etl::string<MAX_INSTRUMENT_NAME_LENGTH> name,
    bool overwrite) {
  auto picoFS = PicoFileSystem::GetInstance();

  // Add .pti extension to the filename
  etl::string<MAX_INSTRUMENT_FILENAME_LENGTH> filename = name;
  filename.append(INSTRUMENT_FILE_EXTENSION);

  etl::vector<const char *, 2> segments = {INSTRUMENTS_DIR, filename.c_str()};
  CreatePath(pathBufferA, segments);

  // check if file already exists
  if (picoFS->exists(pathBufferA.c_str())) {
    if (!overwrite) {
      return PERSIST_EXISTS;
    }
    // Delete the existing file if overwrite is true
    if (!picoFS->DeleteFile(pathBufferA.c_str())) {
      Trace::Error("PERSISTENCYSERVICE: Failed to delete existing file: %s",
                   pathBufferA.c_str());
      return PERSIST_ERROR;
    }
  }

  auto fp = picoFS->Open(pathBufferA.c_str(), "w");
  if (!fp) {
    Trace::Error("PERSISTENCYSERVICE: Could not open file for writing: %s",
                 pathBufferA.c_str());
    return PERSIST_ERROR;
  }

  tinyxml2::XMLPrinter printer(fp);

  // Use the instrument's Persistent interface to save its data
  instrument->Save(&printer);

  fp->Close();
  return PERSIST_SAVED;
}

InstrumentType PersistencyService::DetectInstrumentType(const char *name) {
  auto picoFS = PicoFileSystem::GetInstance();

  if (!picoFS->chdir(INSTRUMENTS_DIR)) {
    Trace::Error(
        "PERSISTENCYSERVICE: Could not change to instruments directory");
    return IT_NONE;
  }

  // Load the XML document
  PersistencyDocument doc;
  if (!doc.Load(name)) {
    Trace::Error("PERSISTENCYSERVICE: Could not parse XML from file: %s", name);
    return IT_NONE;
  }

  // Find the INSTRUMENT element
  bool elem = doc.FirstChild();
  if (!elem || strcmp(doc.ElemName(), "INSTRUMENT")) {
    Trace::Error(
        "PERSISTENCYSERVICE: Could not find INSTRUMENT node in file: %s", name);
    return IT_NONE;
  }

  // Check for TYPE attribute in the INSTRUMENT element
  InstrumentType importedType = IT_NONE;
  bool hasAttr = doc.NextAttribute();
  while (hasAttr) {
    if (!strcasecmp(doc.attrname_, "TYPE")) {
      Trace::Log("PERSISTENCYSERVICE", "Found instrument type in XML: %s",
                 doc.attrval_);

      // Map the type string to InstrumentType enum
      for (int i = 0; i < IT_LAST; i++) {
        if (!strcasecmp(doc.attrval_, InstrumentTypeNames[i])) {
          importedType = static_cast<InstrumentType>(i);
          Trace::Log("PERSISTENCYSERVICE", "Mapped to instrument type: %d",
                     importedType);
          break;
        }
      }
    }
    hasAttr = doc.NextAttribute();
  }
  return importedType;
}

PersistencyResult PersistencyService::ImportInstrument(I_Instrument *instrument,
                                                       const char *name) {
  auto picoFS = PicoFileSystem::GetInstance();

  if (!picoFS->chdir(INSTRUMENTS_DIR)) {
    Trace::Error(
        "PERSISTENCYSERVICE: Could not change to instruments directory");
    return PERSIST_ERROR;
  }

  // Load the XML document
  PersistencyDocument doc;
  if (!doc.Load(name)) {
    Trace::Error("PERSISTENCYSERVICE: Could not parse XML from file: %s", name);
    return PERSIST_ERROR;
  }

  // Find the INSTRUMENT element
  bool elem = doc.FirstChild();
  if (!elem || strcmp(doc.ElemName(), "INSTRUMENT")) {
    Trace::Error(
        "PERSISTENCYSERVICE: Could not find INSTRUMENT node in file: %s", name);
    return PERSIST_ERROR;
  }

  // Check for TYPE attribute in the INSTRUMENT element
  InstrumentType importedType = IT_NONE;
  bool hasAttr = doc.NextAttribute();
  while (hasAttr) {
    if (!strcasecmp(doc.attrname_, "TYPE")) {
      Trace::Log("PERSISTENCYSERVICE", "Found instrument type in XML: %s",
                 doc.attrval_);

      // Map the type string to InstrumentType enum
      for (int i = 0; i < IT_LAST; i++) {
        if (!strcasecmp(doc.attrval_, InstrumentTypeNames[i])) {
          importedType = static_cast<InstrumentType>(i);
          Trace::Log("PERSISTENCYSERVICE", "Mapped to instrument type: %d",
                     importedType);
          break;
        }
      }
    }
    hasAttr = doc.NextAttribute();
  }

  // If we found a valid instrument type and it doesn't match the current
  // instrument
  if (importedType != IT_NONE && importedType != instrument->GetType()) {
    Trace::Log("PERSISTENCYSERVICE",
               "Current instrument type: %d, imported type: %d",
               instrument->GetType(), importedType);

    // We can't directly change the instrument type, so we'll need to
    // create a new instrument of the correct type and copy parameters later
    Trace::Log("PERSISTENCYSERVICE",
               "Instrument type mismatch, will convert after loading");
  }

  // Restore the instrument content
  if (!instrument->Restore(&doc)) {
    Trace::Error(
        "PERSISTENCYSERVICE: Failed to restore instrument from file: %s", name);
    return PERSIST_ERROR;
  }

  // Extract instrument name from filename (minus .pti extension)
  etl::string<MAX_INSTRUMENT_NAME_LENGTH> instrumentName;
  const char *dotPos = strrchr(name, '.');
  if (dotPos) {
    // Calculate the length of the name without extension
    size_t nameLength = dotPos - name;
    // Copy only up to MAX_INSTRUMENT_NAME_LENGTH characters
    nameLength = nameLength < MAX_INSTRUMENT_NAME_LENGTH
                     ? nameLength
                     : MAX_INSTRUMENT_NAME_LENGTH - 1;
    instrumentName.assign(name, nameLength);
  } else {
    // No extension found, use the whole name (up to MAX_INSTRUMENT_NAME_LENGTH)
    instrumentName.assign(name, strlen(name) < MAX_INSTRUMENT_NAME_LENGTH
                                    ? strlen(name)
                                    : MAX_INSTRUMENT_NAME_LENGTH - 1);
  }

  // Set the instrument name
  Variable *nameVar = instrument->FindVariable(FourCC::InstrumentName);
  if (nameVar) {
    nameVar->SetString(instrumentName.c_str());
  }

  // Mark the instrument as changed to trigger UI updates
  instrument->SetChanged();
  instrument->NotifyObservers();

  Trace::Log("PERSISTENCYSERVICE", "Successfully imported instrument settings");
  Trace::Log("PERSISTENCYSERVICE", "Set instrument name to: %s",
             instrumentName.c_str());
  return PERSIST_LOADED;
}
