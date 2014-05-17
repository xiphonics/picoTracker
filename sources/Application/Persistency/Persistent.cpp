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

bool Persistent::Restore(TiXmlElement *element) {
	if (!strcmp(element->Value(),nodeName_)) {
		RestoreContent(element) ;
		return true ;
	}
	return false ;
} ;
