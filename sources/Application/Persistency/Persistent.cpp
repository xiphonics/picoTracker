#include "Persistent.h"
#include "Foundation/Types/Types.h"

Persistent::Persistent(const char *nodeName)
    : SubService(FourCC::ServicePersistency) {
  nodeName_ = nodeName;
};

void Persistent::Save(tinyxml2::XMLPrinter *printer) {
  printer->OpenElement(nodeName_);
  SaveContent(printer);
  printer->CloseElement();
};

bool Persistent::Restore(PersistencyDocument *doc) {
  if (!strcmp(doc->ElemName(), nodeName_)) {
    RestoreContent(doc);
    return true;
  }
  return false;
};
