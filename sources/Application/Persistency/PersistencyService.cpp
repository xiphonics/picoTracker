#include "PersistencyService.h"
#include "Persistent.h"
#include "System/Console/Trace.h"
#include "Foundation/Types/Types.h"

PersistencyService::PersistencyService():Service(MAKE_FOURCC('S','V','P','S')) {
} ;

void PersistencyService::Save() {

	Path filename("project:lgptsav.dat") ;

	TiXmlDocument doc(filename.GetPath().c_str());
	TiXmlElement first("LITTLEGPTRACKER") ;
	TiXmlNode *node=doc.InsertEndChild(first) ;

	// Loop on all registered service
	// accumulating XML flow
	
	IteratorPtr<SubService> it(GetIterator()) ;
	for (it->Begin();!it->IsDone();it->Next()) {
		Persistent *currentItem=(Persistent *)&it->CurrentItem() ;
		currentItem->Save(node) ;
	} ;

	doc.SaveFile() ;
} ;

bool PersistencyService::Load() {
  Path filename("project:lgptsav.dat");
  PersistencyDocument doc;
  if (!doc.Load(filename.GetPath())) return false;

  bool elem = doc.FirstChild(); // advance to first child
  if (!elem || strcmp(doc.ElemName(), "LITTLEGPTRACKER")) {
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
