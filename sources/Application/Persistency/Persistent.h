/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _PERSISTENT_H_
#define _PERSISTENT_H_

#include "Application/Persistency/PersistencyDocument.h"
#include "Externals/TinyXML2/tinyxml2.h"
#include "Foundation/Services/SubService.h"

class Persistent : SubService {
public:
  Persistent(const char *nodeName);
  void Save(tinyxml2::XMLPrinter *printer);
  bool Restore(PersistencyDocument *doc);

protected:
  virtual void SaveContent(tinyxml2::XMLPrinter *printer) = 0;
  virtual void RestoreContent(PersistencyDocument *doc) = 0;

private:
  const char *nodeName_;
};

#endif
