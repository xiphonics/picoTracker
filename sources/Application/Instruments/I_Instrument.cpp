#include "I_Instrument.h"
#include "Application/Utils/char.h"

I_Instrument::~I_Instrument() {
  // Virtual destructor implementation
}

void I_Instrument::SaveContent(tinyxml2::XMLPrinter *printer) {
  // Save the instrument type
  printer->PushAttribute("TYPE", InstrumentTypeNames[GetType()]);

  // Save all the instrument's parameters
  for (auto it = Variables()->begin(); it != Variables()->end(); it++) {
    printer->OpenElement("PARAM");
    printer->PushAttribute("NAME", (*it)->GetName());
    printer->PushAttribute("VALUE", (*it)->GetString().c_str());
    printer->CloseElement(); // PARAM
  }
}

void I_Instrument::RestoreContent(PersistencyDocument *doc) {
  // Restore parameters from the document
  bool elem = doc->FirstChild();
  while (elem) {
    if (!strcasecmp(doc->ElemName(), "PARAM")) {
      const char *name = nullptr;
      const char *value = nullptr;

      bool hasAttr = doc->NextAttribute();
      while (hasAttr) {
        if (!strcasecmp(doc->attrname_, "NAME")) {
          name = doc->attrval_;
        }
        if (!strcasecmp(doc->attrname_, "VALUE")) {
          value = doc->attrval_;
        }
        hasAttr = doc->NextAttribute();
      }

      if (name && value) {
        // Find the variable with this name and set its value
        for (auto it = Variables()->begin(); it != Variables()->end(); it++) {
          if (!strcasecmp((*it)->GetName(), name)) {
            (*it)->SetString(value);
            break;
          }
        }
      }
    }
    elem = doc->NextSibling();
  }
}
