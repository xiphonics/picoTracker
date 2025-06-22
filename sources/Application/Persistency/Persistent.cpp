/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

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
