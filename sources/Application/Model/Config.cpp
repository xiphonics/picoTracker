#include "Config.h"
#include "Application/Persistency/PersistencyDocument.h"
#include "Externals/etl/include/etl/flat_map.h"
#include "Externals/etl/include/etl/string.h"
#include "Externals/etl/include/etl/string_utilities.h"

#define CONFIG_FILE_PATH "/.config.xml"
#define CONFIG_VERSION_NUMBER 1

static const char *lineOutOptions[3] = {"Line level", "HP High", "HP Low"};

// Param keys MUST fit in this length limit!
typedef etl::string<10> ParamString;

static const etl::flat_map validParameters{
    etl::pair{ParamString("BACKGROUND"), ParamString("0F0F0F")},
    etl::pair{ParamString("FOREGROUND"), ParamString("ADADAD")},
    etl::pair{ParamString("HICOLOR1"), ParamString("846F94")},
    etl::pair{ParamString("HICOLOR2"), ParamString("6B316B")},
    etl::pair{ParamString("CONSOLECOLOR"), ParamString("99FFAA")},
    etl::pair{ParamString("CURSORCOLOR"), ParamString("224400")},
    etl::pair{ParamString("INFOCOLOR"), ParamString("33EE33")},
    etl::pair{ParamString("WARNCOLOR"), ParamString("11EE22")},
    etl::pair{ParamString("ERRORCOLOR"), ParamString("FF1111")},
    etl::pair{ParamString("LINEOUT"), ParamString("2")},
};

Config::Config() : lineOut_("lineout", VAR_LINEOUT, lineOutOptions, 3, 0) {
  PersistencyDocument doc;

  if (!doc.Load(CONFIG_FILE_PATH)) {
    Trace::Error("CONFIG: Could not open file for reading: %s",
                 CONFIG_FILE_PATH);
    useDefaultConfig();
    return;
  }

  bool elem = doc.FirstChild();
  if (!elem || strcmp(doc.ElemName(), "CONFIG")) {
    Trace::Log("CONFIG", "Bad config.xml format!");
    // TODO: need show user some UI that config file is invalid
    useDefaultConfig();
    return;
  }
  elem = doc.FirstChild(); // now get first child element of CONFIG
  while (elem) {
    if (!validParameters.contains(ParamString(doc.ElemName()))) {
      Trace::Log("CONFIG", "Found unknown config parameter \"%s\", skipping...",
                 doc.ElemName());
      break;
    }
    bool hasAttr = doc.NextAttribute();
    while (hasAttr) {
      if (!strcasecmp(doc.attrname_, "VALUE")) {
        processParams(doc.ElemName(), doc.attrval_);
        Trace::Log("CONFIG", "Read Param:%s->[%s]", doc.ElemName(),
                   doc.attrval_);
      }
      hasAttr = doc.NextAttribute();
    }
    elem = doc.NextSibling();
  }
  Trace::Log("CONFIG", "Loaded successfully");
}

Config::~Config() {}

void Config::Save() {
  auto picoFS = PicoFileSystem::GetInstance();
  PI_File *fp = picoFS->Open(CONFIG_FILE_PATH, "w");
  if (!fp) {
    Trace::Error("CONFIG: Could not open file for writing: %s",
                 CONFIG_FILE_PATH);
  }
  Trace::Log("PERSISTENCYSERVICE", "Opened Proj File: %s\n", CONFIG_FILE_PATH);
  tinyxml2::XMLPrinter printer(fp);

  SaveContent(&printer);

  fp->Close();
}

void Config::SaveContent(tinyxml2::XMLPrinter *printer) {
  // store config version
  printer->OpenElement("CONFIG");
  printer->PushAttribute("VERSION", CONFIG_VERSION_NUMBER);

  // save all of the config parameters
  auto it = begin();
  for (size_t i = 0; i < size(); i++) {
    etl::string<16> elemName = (*it)->GetName();
    to_upper_case(elemName);
    printer->OpenElement(elemName.c_str());
    if (elemName.compare("LINEOUT") == 0) {
      printer->PushAttribute("VALUE", std::to_string((*it)->GetInt()).c_str());
    } else {
      printer->PushAttribute("VALUE", (*it)->GetString().c_str());
    }
    printer->CloseElement();
    it++;
  }
  printer->CloseElement();
  Trace::Log("CONFIG", "Saved config");
};

const char *Config::GetValue(const char *key) {
  Variable *v = FindVariable(key);
  if (v) {
    Trace::Log("CONFIG", "Got value for %s=%s", key, v->GetString().c_str());
  } else {
    Trace::Log("CONFIG", "No value for requested key:%s", key);
  }
  return v ? v->GetString().c_str() : 0;
};

// need to handle some variable like LINEOUT separately
void Config::processParams(const char *name, const char *value) {
  if (!strcmp(name, "LINEOUT")) {
    lineOut_.SetInt(std::stoi(value));
    insert(end(), &lineOut_);
  } else {
    Variable *v = new Variable(name, 0, value);
    insert(end(), v);
  }
}

void Config::useDefaultConfig() {
  Trace::Log("CONFIG", "Setting DEFAULT values for config parameters");
  for (auto const &p : validParameters) {
    processParams(p.first.c_str(), p.second.c_str());
  }
  Save(); // and write the defaults to SDCard
}