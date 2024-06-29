#include "PersistencyService.h"
#include "Foundation/Types/Types.h"
#include "Persistent.h"
#include "System/Console/Trace.h"
#include <etl/string.h>

PersistencyService::PersistencyService()
    : Service(MAKE_FOURCC('S', 'V', 'P', 'S')){};

void PersistencyService::Save(const char *projectName) {
  etl::string<128> projectFilePath("/projects/");
  projectFilePath.append(projectName);

  // TODO: Check if proj dir exists:
  auto picoFS = PicoFileSystem::GetInstance();
  if (!picoFS->exists(projectFilePath.c_str())) {
    // need to first create project dir
    picoFS->makeDir(projectFilePath.c_str());
  }
  projectFilePath.append("/lgptsav.dat");

  PI_File *fp = picoFS->Open(projectFilePath.c_str(), "w");
  if (!fp) {
    Trace::Error("Could not open file for writing: %s",
                 projectFilePath.c_str());
  }
  Trace::Log("PERSISTENCYSERVICE", "Opened Proj File: %s\n",
             projectFilePath.c_str());
  tinyxml2::XMLPrinter printer(fp);
  Trace::Log("PERSISTENCYSERVICE", "Saved Proj File: %s\n",
             projectFilePath.c_str());

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
};

bool PersistencyService::Load(const char *projectName) {
  etl::string<128> projectFilePath("/projects/");
  projectFilePath.append(projectName);
  projectFilePath.append("/lgptsav.dat");

  PersistencyDocument doc;
  if (!doc.Load(projectFilePath.c_str()))
    return false;

  bool elem = doc.FirstChild(); // advance to first child
  if (!elem || strcmp(doc.ElemName(), "PICOTRACKER")) {
    Trace::Error("could not find master node");
    return false;
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
  return true;
};
