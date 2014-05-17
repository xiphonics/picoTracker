#include "Config.h"


Config::Config() 
{
	Path path("bin:config.xml") ;
	Trace::Log("CONFIG","Got config path=%s",path.GetPath().c_str()) ;
	TiXmlDocument *document=new TiXmlDocument(path.GetPath());
	bool loadOkay = document->LoadFile();

	if (loadOkay) 
  { 
		// Check first node is CONFIG/ GPCONFIG

		TiXmlNode* rootnode = 0;

		rootnode = document->FirstChild( "CONFIG" );
		if (!rootnode)
    {
		   rootnode = document->FirstChild( "GPCONFIG" );
    }
    
		if (rootnode)
    {
			TiXmlElement *rootelement = rootnode->ToElement();
			TiXmlNode *node = rootelement->FirstChildElement() ;

			// Loop on all children
		
			if (node)
      {
				TiXmlElement *element = node->ToElement();
				while (element) 
        {
					const char *key=element->Value() ;
					const char *value=element->Attribute("value") ;
					if (!value)
          {
						value=element->Attribute("VALUE") ;
					}
					if (key&&value)
          {
						Variable *v=new Variable(key,0,value) ;
						Insert(v) ;
					}
					element = element->NextSiblingElement(); 
				}
			}

    }
    } else {
		Trace::Log("CONFIG","No (bad?) config.xml") ;
	}
 	delete(document) ;
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
