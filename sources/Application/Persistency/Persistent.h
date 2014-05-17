#ifndef _PERSISTENT_H_
#define _PERSISTENT_H_

#include "Foundation/Services/SubService.h"
#include "Externals/TinyXML/tinyxml.h"

class Persistent:SubService {
public:
	Persistent(const char *nodeName) ;
	void Save(TiXmlNode *node) ;
	bool Restore(TiXmlElement *element) ;
protected:
	virtual void SaveContent(TiXmlNode *node)=0 ;
	virtual void RestoreContent(TiXmlElement *element)=0 ;
private:
	const char *nodeName_ ;
} ;

#endif
