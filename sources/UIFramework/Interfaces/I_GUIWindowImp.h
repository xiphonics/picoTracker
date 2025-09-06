/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _I_GUIWINDOWIMP_H_
#define _I_GUIWINDOWIMP_H_

#include "I_GUIGraphics.h"
#include "UIFramework/BasicDatas/GUICreateWindowParams.h"
#include "UIFramework/BasicDatas/GUIEvent.h"

class I_GUIWindowFactory; // forward declaration
class GUIWindow;

// Base class for implementation window. Base operation directed
// to system calls are passed from a GUIWindow to an instance of
// system class derived from this interface

class I_GUIWindowImp : public I_GUIGraphics {
public:
  virtual ~I_GUIWindowImp(){};

  // This method allows to have back pointer to the framework window
  // to avoid heavy searches when having to forward system events

  void SetWindow(GUIWindow &);

  virtual void PushEvent(GUIEvent &) = 0;
  virtual void DrawRect(uint8_t colorIdx, GUIRect &r) = 0;

  //	virtual void Save()=0 ;
  //	virtual void Restore()=0 ;

protected:
  GUIWindow *_window; // The GUIWindow associated to the Imp.
};
#endif
