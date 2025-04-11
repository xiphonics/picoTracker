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
  Trace::Log("I_INSTRUMENT",
             "RestoreContent: Starting to restore instrument parameters");

  // First, check for TYPE attribute in the INSTRUMENT element
  bool hasAttr = doc->NextAttribute();
  while (hasAttr) {
    if (!strcasecmp(doc->attrname_, "TYPE")) {
      Trace::Log("I_INSTRUMENT", "Instrument type from XML: %s", doc->attrval_);
      // TODO: We already know the instrument type so need to validate it
      // matches the imported one here
    }
    hasAttr = doc->NextAttribute();
  }

  // Navigate to the first child of the INSTRUMENT element (which should be a
  // PARAM element)
  bool subelem = doc->FirstChild();
  int paramCount = 0;

  while (subelem) {
    // Process the PARAM element attributes
    bool hasAttr = doc->NextAttribute();
    char name[24] = "";
    char value[24] = "";

    while (hasAttr) {
      if (!strcasecmp(doc->attrname_, "NAME")) {
        strcpy(name, doc->attrval_);
      }
      if (!strcasecmp(doc->attrname_, "VALUE")) {
        strcpy(value, doc->attrval_);
      }
      hasAttr = doc->NextAttribute();
    }

    if (name[0] != '\0' && value[0] != '\0') {
      // Trace::Log("I_INSTRUMENT", "Found parameter in XML: %s = %s", name,
      //            value);
      bool found = false;

      // Find the variable with this name and set its value
      for (auto it = Variables()->begin(); it != Variables()->end(); it++) {
        if (!strcasecmp((*it)->GetName(), name)) {
          (*it)->SetString(value);
          // Trace::Log("I_INSTRUMENT", "Set parameter: %s = %s", name, value);
          found = true;
          paramCount++;
          break;
        }
      }

      if (!found) {
        Trace::Error("Parameter '%s' not found in instrument", name);
      }
    }

    // Move to the next PARAM element
    subelem = doc->NextSibling();
  }

  Trace::Log("I_INSTRUMENT", "RestoreContent: Restored %d parameters",
             paramCount);
}
