#include "Persistent.h"
#include "Foundation/Types/Types.h"

Persistent::Persistent(const char *nodeName):SubService(MAKE_FOURCC('S','V','P','S')) {
	nodeName_=nodeName ;
} ;

void Persistent::Save(TiXmlNode *node) {
	TiXmlElement master(nodeName_) ;
	TiXmlNode *first=node->InsertEndChild(master) ;
	SaveContent(first) ;
} ;

bool Persistent::Restore(PersistencyDocument *doc) {
  if (!strcmp(doc->ElemName(), nodeName_)) {
    RestoreContent(doc);
    return true;
  }
  return false;
};
