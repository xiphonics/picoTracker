#include "Config.h"
#include "Application/Persistency/PersistencyDocument.h"
#include "Externals/etl/include/etl/flat_map.h"
#include "Externals/etl/include/etl/string.h"
#include "Externals/etl/include/etl/string_utilities.h"
#include "Services/Midi/MidiService.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/I_File.h"
#include "Variable.h"
#include <stdlib.h>

#define CONFIG_FILE_PATH "/.config.xml"
#define CONFIG_VERSION_NUMBER 1

#define MIDI_DEVICE_LEN 4

static const char *lineOutOptions[3] = {"HP Low", "HP High", "Line Level"};
static const char *midiDeviceList[MIDI_DEVICE_LEN] = {"OFF", "TRS", "USB",
                                                      "TRS+USB"};
static const char *midiSendSync[2] = {"Off", "Send"};
static const char *midiClockSyncOptions[2] = {"Internal", "External"};
static const char *remoteUIOnOff[2] = {"Off", "On"};

static const char *fontOptions[2] = {"Standard", "Bold"};

// Param keys MUST fit in this length limit!
typedef etl::string<13> ParamString;

// Define default values at compile time
constexpr int DEFAULT_BACKGROUND = 0x0F0F0F;
constexpr int DEFAULT_FOREGROUND = 0xADADAD;
constexpr int DEFAULT_HICOLOR1 = 0x846F94;
constexpr int DEFAULT_HICOLOR2 = 0x6B316B;
constexpr int DEFAULT_CONSOLECOLOR = 0xFF00FF;
constexpr int DEFAULT_CURSORCOLOR = 0x776B56;
constexpr int DEFAULT_INFOCOLOR = 0x29EE3D;
constexpr int DEFAULT_WARNCOLOR = 0xEFFA52;
constexpr int DEFAULT_ERRORCOLOR = 0xE84D15;
constexpr int DEFAULT_PLAYCOLOR = 0x00FF00;
constexpr int DEFAULT_MUTECOLOR = 0xFF0000;
constexpr int DEFAULT_SONGVIEWFECOLOR = 0xFFA500;
constexpr int DEFAULT_SONGVIEW00COLOR = 0x0000FF;
constexpr int DEFAULT_ROWCOLOR = 0x555555;
constexpr int DEFAULT_ROW2COLOR = 0x777777;
constexpr int DEFAULT_MAJORBEATCOLOR = 0xFFFF00;
constexpr int DEFAULT_LINEOUT = 0x2;
constexpr int DEFAULT_MIDIDEVICE = 0x0;
constexpr int DEFAULT_MIDISYNC = 0x0;
constexpr int DEFAULT_MIDICLOCKSYNC = 0x0;
constexpr int DEFAULT_REMOTEUI = 0x1;
constexpr int DEFAULT_UIFONT = 0x0;

// Use a struct to define parameter information
struct ConfigParam {
  const char *name;
  int defaultValue;
  FourCC::enum_type fourcc;
};

// Define parameters as a static array instead of a ETL flat_map for example,
// because using a flat_map static requires too much stack space for
// initialization
static const ConfigParam configParams[] = {
    {"BACKGROUND", DEFAULT_BACKGROUND, FourCC::VarBGColor},
    {"FOREGROUND", DEFAULT_FOREGROUND, FourCC::VarFGColor},
    {"HICOLOR1", DEFAULT_HICOLOR1, FourCC::VarHI1Color},
    {"HICOLOR2", DEFAULT_HICOLOR2, FourCC::VarHI2Color},
    {"CONSOLECOLOR", DEFAULT_CONSOLECOLOR, FourCC::VarConsoleColor},
    {"CURSORCOLOR", DEFAULT_CURSORCOLOR, FourCC::VarCursorColor},
    {"INFOCOLOR", DEFAULT_INFOCOLOR, FourCC::VarInfoColor},
    {"WARNCOLOR", DEFAULT_WARNCOLOR, FourCC::VarWarnColor},
    {"ERRORCOLOR", DEFAULT_ERRORCOLOR, FourCC::VarErrorColor},
    {"PLAYCOLOR", DEFAULT_PLAYCOLOR, FourCC::VarPlayColor},
    {"MUTECOLOR", DEFAULT_MUTECOLOR, FourCC::VarMuteColor},
    {"SONGVIEWFECOLOR", DEFAULT_SONGVIEWFECOLOR, FourCC::VarSongViewFEColor},
    {"SONGVIEW00COLOR", DEFAULT_SONGVIEW00COLOR, FourCC::VarSongView00Color},
    {"ROWCOLOR", DEFAULT_ROWCOLOR, FourCC::VarRowColor},
    {"ROW2COLOR", DEFAULT_ROW2COLOR, FourCC::VarRow2Color},
    {"MAJORBEATCOLOR", DEFAULT_MAJORBEATCOLOR, FourCC::VarMajorBeatColor},
    {"LINEOUT", DEFAULT_LINEOUT, FourCC::VarLineOut},
    {"MIDIDEVICE", DEFAULT_MIDIDEVICE, FourCC::VarMidiDevice},
    {"MIDISYNC", DEFAULT_MIDISYNC, FourCC::VarMidiSync},
    {"MIDICLOCKSYNC", DEFAULT_MIDICLOCKSYNC, FourCC::VarMidiClockSync},
    {"REMOTEUI", DEFAULT_REMOTEUI, FourCC::VarRemoteUI},
    {"UIFONT", DEFAULT_UIFONT, FourCC::VarUIFont},
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
    // Check if the parameter exists in our parameters list
    bool paramFound = false;
    for (const auto &param : configParams) {
      if (strcmp(doc.ElemName(), param.name) == 0) {
        paramFound = true;
        break;
      }
    }

    if (!paramFound) {
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

bool Config::Save() {
  auto fs = FileSystem::GetInstance();
  I_File *fp = fs->Open(CONFIG_FILE_PATH, "w");
  if (!fp) {
    Trace::Error("CONFIG: Could not open file for writing: %s",
                 CONFIG_FILE_PATH);
  }
  Trace::Log("PERSISTENCYSERVICE", "Opened Proj File: %s", CONFIG_FILE_PATH);
  tinyxml2::XMLPrinter printer(fp);

  SaveContent(&printer);

  return fp->Close();
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
    if ((*it)->GetType() == Variable::CHAR_LIST) {
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
    } else if (!strcmp(name, FourCC(FourCC::VarPlayColor).c_str())) {
      fourcc = FourCC::VarPlayColor;
    } else if (!strcmp(name, FourCC(FourCC::VarMuteColor).c_str())) {
      fourcc = FourCC::VarMuteColor;
    } else if (!strcmp(name, FourCC(FourCC::VarSongViewFEColor).c_str())) {
      fourcc = FourCC::VarSongViewFEColor;
    } else if (!strcmp(name, FourCC(FourCC::VarSongView00Color).c_str())) {
      fourcc = FourCC::VarSongView00Color;
    } else if (!strcmp(name, FourCC(FourCC::VarRowColor).c_str())) {
      fourcc = FourCC::VarRowColor;
    } else if (!strcmp(name, FourCC(FourCC::VarRow2Color).c_str())) {
      fourcc = FourCC::VarRow2Color;
    } else if (!strcmp(name, FourCC(FourCC::VarMajorBeatColor).c_str())) {
      fourcc = FourCC::VarMajorBeatColor;
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

  // Process all parameters from the static array
  for (const auto &param : configParams) {
    processParams(param.name, param.defaultValue, true);
  }
}
