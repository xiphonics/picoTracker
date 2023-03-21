#include "Config.h"
#include "Application/Persistency/PersistencyDocument.h"
#include "System/FileSystem/FileSystem.h"
#include <cstring>

Config::Config() 
{
	Path path("bin:config.xml") ;
	Trace::Log("CONFIG","Got config path=%s",path.GetPath().c_str()) ;
  PersistencyDocument doc;
  if (!doc.Load(path.GetPath())) {
    Trace::Log("CONFIG", "No config.xml");
    return;
  }
  bool elem = doc.FirstChild();
  if (!elem || strcmp(doc.ElemName(), "CONFIG")) {
    Trace::Log("CONFIG", "Bad config.xml");
    return;
  }

  bool validElem;
  elem = doc.FirstChild();
  while (elem) {
    validElem = true;
    if (strcmp(doc.ElemName(), "BACKGROUND") &&
        strcmp(doc.ElemName(), "FOREGROUND") &&
        strcmp(doc.ElemName(), "HICOLOR1") &&
        strcmp(doc.ElemName(), "HICOLOR2")) {
      Trace::Log("CONFIG", "Found unknown config parameter \"%s\", skipping...", doc.ElemName());
      validElem = false;
    }
    bool hasAttr = doc.NextAttribute();
    while (hasAttr) {
      if (!strcmp(doc.attrname_, "value")) {
        if (validElem) {
          Variable *v = new Variable(doc.ElemName(), 0, doc.attrval_);
          Insert(v);
        }
      }
      hasAttr = doc.NextAttribute();
    }
    elem = doc.NextSibling();
  }
  Trace::Log("CONFIG", "Loaded successfully");
}

//------------------------------------------------------------------------------

Config::~Config()
{
}


//------------------------------------------------------------------------------

const char *Config::GetValue(const char *key) 
{
	Variable *v=FindVariable(key) ;
	if (v) {
		Trace::Log("CONFIG","Got value for %s=%s",key,v->GetString()) ;
	}
	return v?v->GetString():0 ;
} ;


//------------------------------------------------------------------------------

void Config::ProcessArguments(int argc,char **argv) 
{
	for (int i=1;i<argc;i++) {
		char *pos ;
		char *arg=argv[i] ;
		while (*arg=='-') arg++ ;
		if ((pos=strchr(arg,'='))!=0) {
			*pos=0 ;
			Variable *v=FindVariable(arg) ;
			if (v) {
				v->SetString(pos+1) ;
			} else {
				Variable *v=new Variable(arg,0,pos+1) ;
				Insert(v) ;
			}
		}
	}
} ;


//------------------------------------------------------------------------------
