/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "System/Console/Trace.h"
// #include "Framework/Types/Types.h"
#include "ControllerService.h"
#include <stdlib.h>
#include <string.h>
#include <string>

using namespace std;

Channel *ControllerServiceBase::GetChannel(const char *sourcePath) {

  string path = sourcePath;

  // Look for the controller source class

  string::size_type pos = path.find(":", 0);
  if (pos == string::npos) {
    return 0;
  };

  // Look for the device name or ID

  string sourceclass = path.substr(0, pos);
  path = path.substr(pos + 1);

  pos = path.find(":", 0);
  if (pos == string::npos) {
    return 0;
  };

  int deviceID = -1; // Default, we search by name
  string deviceName = path.substr(0, pos);
  bool isDigit = true;
  for (unsigned int i = 0; i < deviceName.length(); i++) {
    unsigned char c = deviceName[i];
    if (!isdigit(c)) {
      isDigit = false;
      break;
    };
  }

  if (isDigit) {
    deviceID = atoi(deviceName.c_str());
  };

  path = path.substr(pos + 1);

  // Now look for a device that matches name & class
  ControllerSource *source = 0;

  for (Begin(); !IsDone(); Next()) {
    ControllerSource &cs = CurrentItem();
    if ((string(cs.GetClass()) == sourceclass) && (cs.IsRunning())) {
      if (deviceID-- == 0) {
        source = &cs;
        break;
      };
      if (!strcmp(deviceName.c_str(), cs.GetName())) {
        source = &cs;
        break;
      };
    };
  };
  if (source == 0)
    return 0;

  return source->GetChannel(path.c_str());
};
