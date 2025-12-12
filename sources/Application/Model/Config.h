/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "Application/Persistency/Persistent.h"
#include "Foundation/T_Singleton.h"
#include "Foundation/Variables/VariableContainer.h"
#include "Foundation/Variables/WatchedVariable.h"
#include "System/Console/Trace.h"

class Config : public T_Singleton<Config>, public VariableContainer {
public:
  Config();
  ~Config();
  int GetValue(const char *key);
  void ProcessArguments(int argc, char **argv);
  bool Save();

  // Methods for handling color variables and themes
  void WriteColorVariables(tinyxml2::XMLPrinter *printer);
  void ReadColorVariable(PersistencyDocument *doc);

  // Theme-related methods (replacing Theme class)
  bool SaveTheme(tinyxml2::XMLPrinter *printer, const char *themeName);
  bool LoadTheme(PersistencyDocument *doc);
  bool ExportTheme(const char *themeName, bool overwrite);
  bool ImportTheme(const char *themeName);

private:
  etl::list<Variable *, 25> variables_;
  // Config variables (kept as members to avoid heap allocation)
  WatchedVariable background_;
  WatchedVariable foreground_;
  WatchedVariable hiColor1_;
  WatchedVariable hiColor2_;
  WatchedVariable consoleColor_;
  WatchedVariable cursorColor_;
  WatchedVariable infoColor_;
  WatchedVariable warnColor_;
  WatchedVariable errorColor_;
  WatchedVariable accentColor_;
  WatchedVariable accentAltColor_;
  WatchedVariable emphasisColor_;
  WatchedVariable lineOut_;
  WatchedVariable midiDevice_;
  WatchedVariable midiSync_;
  WatchedVariable remoteUI_;
  WatchedVariable uiFont_;
  Variable themeName_;
  WatchedVariable backlightLevel_;
  WatchedVariable outputVolume_;
  WatchedVariable recordSource_;
  WatchedVariable recordLineGain_;
  WatchedVariable recordMicGain_;

  void SaveContent(tinyxml2::XMLPrinter *printer);
  void useDefaultConfig();
};

#endif
