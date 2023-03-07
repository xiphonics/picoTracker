#ifndef _PERSISTENT_H_
#define _PERSISTENT_H_

#include "Application/Persistency/PersistencyDocument.h"
#include "Externals/TinyXML/tinyxml.h"
#include "Foundation/Services/SubService.h"

class Persistent:SubService {
public:
	Persistent(const char *nodeName) ;
	void Save(TiXmlNode *node) ;
  bool Restore(PersistencyDocument *doc);

protected:
	virtual void SaveContent(TiXmlNode *node)=0 ;
  virtual void RestoreContent(PersistencyDocument *doc) = 0;

private:
	const char *nodeName_ ;
} ;

#endif
