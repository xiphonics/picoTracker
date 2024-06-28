#include "PersistencyService.h"
#include "Foundation/Types/Types.h"
#include "Persistent.h"
#include "System/Console/Trace.h"
#include <etl/string.h>

PersistencyService::PersistencyService()
    : Service(MAKE_FOURCC('S', 'V', 'P', 'S')){};

void PersistencyService::Save() {
  etl::string<128> projectFilePath("/projects/");
  projectFilePath.append(projectName_);
  projectFilePath.append("/lgptsav.dat");

  PI_File *fp =
      PicoFileSystem::GetInstance()->Open(projectFilePath.c_str(), "w");
  printf("Save Proj File: %s\n", projectFilePath.c_str());
  if (!fp) {
    Trace::Error("Could not open file for writing: %s",
                 projectFilePath.c_str());
  }
  tinyxml2::XMLPrinter printer(fp);

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

  strcpy(projectName_, projectName);

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
