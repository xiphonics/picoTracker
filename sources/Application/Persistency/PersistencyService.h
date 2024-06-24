#ifndef _PERSISTENCY_SERVICE_H_
#define _PERSISTENCY_SERVICE_H_

#include "Externals/TinyXML2/tinyxml2.h"
#include "Externals/yxml/yxml.h"
#include "Foundation/Services/Service.h"
#include "Foundation/T_Singleton.h"

#define MAX_PROJECT_NAME_LENGTH 24

class PersistencyService : public Service,
                           public T_Singleton<PersistencyService> {
public:
  PersistencyService();
  void Save();
  bool Load(const char *projectName);

  // TODO: we need to centralise keeping the project name in a single
  //  service but for now we just hack to keep the project name cached
  //  here when loading so that we have it when we come to do Save()
private:
  char projectName_[MAX_PROJECT_NAME_LENGTH];
};

#endif
