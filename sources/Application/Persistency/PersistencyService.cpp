#include "PersistencyService.h"
#include "Persistent.h"
#include "System/Console/Trace.h"
#include "Foundation/Types/Types.h"

PersistencyService::PersistencyService():Service(MAKE_FOURCC('S','V','P','S')) {
} ;

void PersistencyService::Save() {

	Path filename("project:lgptsav.dat") ;
  I_File *fp = FileSystem::GetInstance()->Open(filename.GetPath().c_str(), "w");
  printf("File: %s\n", filename.GetPath().c_str());
  if (!fp) {
    Trace::Error("Could not open file for writing: %s", filename.GetPath().c_str());
  }
  tinyxml2::XMLPrinter printer(fp);

  printer.OpenElement("LITTLEGPTRACKER");

	// Loop on all registered service
	// accumulating XML flow	
	IteratorPtr<SubService> it(GetIterator()) ;
	for (it->Begin();!it->IsDone();it->Next()) {
		Persistent *currentItem=(Persistent *)&it->CurrentItem() ;
		currentItem->Save(&printer);
	} ;

  printer.CloseElement();

  fp->Close();
  delete (fp);
};

bool PersistencyService::Load() {
  Path filename("project:lgptsav.dat");
  PersistencyDocument doc;
  if (!doc.Load(filename.GetPath())) return false;

  bool elem = doc.FirstChild(); // advance to first child
  if (!elem || strcmp(doc.ElemName(), "PICOTRACKER")) {
    Trace::Error("could not find master node");
    return false;
  }

  elem = doc.FirstChild();
  while (elem) {
    IteratorPtr<SubService> it(GetIterator());
    for (it->Begin(); !it->IsDone(); it->Next()) {
      Persistent *currentItem = (Persistent *)&it->CurrentItem();
      if (currentItem->Restore(&doc)) {
        break;
      };
    }
    elem = doc.NextSibling();
  }
  return true;
};
