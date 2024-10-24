#include "Config.h"
#include "Application/Persistency/PersistencyDocument.h"
#include "Externals/etl/include/etl/flat_map.h"
#include "Externals/etl/include/etl/string.h"
#include "Externals/etl/include/etl/string_utilities.h"
#include "Services/Midi/MidiService.h"
#include "Variable.h"
#include <stdlib.h>

#define CONFIG_FILE_PATH "/.config.xml"
#define CONFIG_VERSION_NUMBER 1

#define MIDI_DEVICE_LEN 4

static const char *lineOutOptions[3] = {"HP Low", "HP High", "Line Level"};
static const char *midiDeviceList[MIDI_DEVICE_LEN] = {"OFF", "TRS", "USB",
                                                      "TRS+USB"};
static const char *midiSendSync[2] = {"Off", "Send"};
static const char *remoteUIOnOff[2] = {"Off", "On"};

static const char *fontOptions[2] = {"Standard", "Bold"};

// Param keys MUST fit in this length limit!
typedef etl::string<13> ParamString;

static const etl::flat_map validParameters{
    etl::pair{ParamString("BACKGROUND"), 0x0F0F0F},
    etl::pair{ParamString("FOREGROUND"), 0xADADAD},
    etl::pair{ParamString("HICOLOR1"), 0x846F94},
    etl::pair{ParamString("HICOLOR2"), 0x6B316B},
    etl::pair{ParamString("CONSOLECOLOR"), 0xFF00FF},
    etl::pair{ParamString("CURSORCOLOR"), 0x776B56},
    etl::pair{ParamString("INFOCOLOR"), 0x29EE3D},
    etl::pair{ParamString("WARNCOLOR"), 0xEFFA52},
    etl::pair{ParamString("ERRORCOLOR"), 0xE84D15},
    etl::pair{ParamString("LINEOUT"), 0x2},
    etl::pair{ParamString("MIDIDEVICE"), 0x0},
    etl::pair{ParamString("MIDISYNC"), 0x0},
    etl::pair{ParamString("REMOTEUI"), 0x0},
    etl::pair{ParamString("UIFONT"), 0x0},
};

Config::Config()
    : VariableContainer(&variables_),
      lineOut_(FourCC::VarLineOut, lineOutOptions, 3, 0),
      midiDevice_(FourCC::VarMidiDevice, midiDeviceList, MIDI_DEVICE_LEN),
      midiSync_(FourCC::VarMidiSync, midiSendSync, 2, 0),
      remoteUI_(FourCC::VarRemoteUI, remoteUIOnOff, 2, 0),
      uiFont_(FourCC::VarUIFont, fontOptions, 2, 0) {
  PersistencyDocument doc;

  // First always just apply all default settings values, these will then be
  // overwritten by values read from config file but if there are any missing
  // from config file, ie. new settings that that have been added to firmware
  // since the version of the firmware that wrote the config file they will get
  // default values
  useDefaultConfig();

  if (!doc.Load(CONFIG_FILE_PATH)) {
    Trace::Error("CONFIG: Could not open file for reading: %s",
                 CONFIG_FILE_PATH);
    Save(); // and write the defaults to SDCard
    return;
  }

  bool elem = doc.FirstChild();
  if (!elem || strcmp(doc.ElemName(), "CONFIG")) {
    Trace::Log("CONFIG", "Bad config.xml format!");
    // TODO: need show user some UI that config file is invalid
    Save(); // and write the defaults to SDCard
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
        processParams(doc.ElemName(), atoi(doc.attrval_), false);
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
  auto it = variables_.begin();
  for (size_t i = 0; i < variables_.size(); i++) {
    etl::string<16> elemName = (*it)->GetName();
    to_upper_case(elemName);
    printer->OpenElement(elemName.c_str());
    // these settings need to be saved as thier Int values not as String values
    // hence we *dont* use GetString() !
    if ((*it)->GetType() == Variable::CHAR_LIST || (*it)->GetType() == Variable::INT) {
      printer->PushAttribute("VALUE", std::to_string((*it)->GetInt()).c_str());
    } else {
      // all other settings need to be saved as thier String values
      printer->PushAttribute("VALUE", (*it)->GetString().c_str());
    }
    printer->CloseElement();
    it++;
  }
  printer->CloseElement();
  Trace::Log("CONFIG", "Saved config");
};

int Config::GetValue(const char *key) {
  Variable *v = FindVariable(key);
  if (v) {
    Trace::Log("CONFIG", "Got value for %s=%s", key, v->GetString().c_str());
  } else {
    Trace::Log("CONFIG", "No value for requested key:%s", key);
  }
  return v ? v->GetInt() : 0;
};

// need to handle some variable like LINEOUT separately
void Config::processParams(const char *name, int value, bool insert) {
  if (!strcmp(name, "LINEOUT")) {
    lineOut_.SetInt(value);
    if (insert) {
      variables_.insert(variables_.end(), &lineOut_);
    }
  } else if (!strcmp(name, "MIDIDEVICE")) {
    midiDevice_.SetInt(value);
    if (insert) {
      variables_.insert(variables_.end(), &midiDevice_);
    }
  } else if (!strcmp(name, "MIDISYNC")) {
    midiSync_.SetInt(value);
    if (insert) {
      variables_.insert(variables_.end(), &midiSync_);
    }
  } else if (!strcmp(name, "REMOTEUI")) {
    remoteUI_.SetInt(value);
    if (insert) {
      variables_.insert(variables_.end(), &remoteUI_);
    }
  } else if (!strcmp(name, "UIFONT")) {
    uiFont_.SetInt(value);
    if (insert) {
      variables_.insert(variables_.end(), &uiFont_);
    }
  } else {
    auto fourcc = FourCC::Default;
    // TODO: need to be able to assign fourcc based on name of element from
    // the Xml config
    if (!strcmp(name, FourCC(FourCC::VarFGColor).c_str())) {
      fourcc = FourCC::VarFGColor;
    } else if (!strcmp(name, FourCC(FourCC::VarBGColor).c_str())) {
      fourcc = FourCC::VarBGColor;
    } else if (!strcmp(name, FourCC(FourCC::VarHI1Color).c_str())) {
      fourcc = FourCC::VarHI1Color;
    } else if (!strcmp(name, FourCC(FourCC::VarHI2Color).c_str())) {
      fourcc = FourCC::VarHI2Color;
    } else if (!strcmp(name, FourCC(FourCC::VarConsoleColor).c_str())) {
      fourcc = FourCC::VarConsoleColor;
    } else if (!strcmp(name, FourCC(FourCC::VarCursorColor).c_str())) {
      fourcc = FourCC::VarCursorColor;
    } else if (!strcmp(name, FourCC(FourCC::VarInfoColor).c_str())) {
      fourcc = FourCC::VarInfoColor;
    } else if (!strcmp(name, FourCC(FourCC::VarWarnColor).c_str())) {
      fourcc = FourCC::VarWarnColor;
    } else if (!strcmp(name, FourCC(FourCC::VarErrorColor).c_str())) {
      fourcc = FourCC::VarErrorColor;
    }
    if (insert) {
      Variable *v = new Variable(fourcc, value);
      variables_.insert(variables_.end(), v);
    } else {
      FindVariable(fourcc)->SetInt(value);
    }
  }
}

void Config::useDefaultConfig() {
  Trace::Log("CONFIG", "Setting DEFAULT values for config parameters");
  for (auto const &p : validParameters) {
    processParams(p.first.c_str(), p.second, true);
  }
}
