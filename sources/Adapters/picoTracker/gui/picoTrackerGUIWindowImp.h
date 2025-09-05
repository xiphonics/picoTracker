/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef PICOTRACKERWINDOWIMP_H_
#define PICOTRACKERWINDOWIMP_H_

#include "Adapters/picoTracker/display/chargfx.h"
#include "Foundation/Observable.h"
#include "UIFramework/Interfaces/I_GUIWindowImp.h"
#include "picoTrackerEventQueue.h"
#include <string>

class picoTrackerGUIWindowImp : public I_GUIWindowImp, public I_Observer {

public:
  picoTrackerGUIWindowImp(GUICreateWindowParams &p);
  virtual ~picoTrackerGUIWindowImp();

public: // I_GUIWindowImp implementation
  virtual void SetColor(GUIColor &) override;
  virtual void DrawRect(GUIRect &) override;
  virtual void DrawChar(const char c, GUIPoint &pos, GUITextProperties &);
  virtual void DrawString(const char *string, GUIPoint &pos,
                          GUITextProperties &, bool overlay = false){};
  virtual GUIRect GetRect();
  virtual void Invalidate();
  virtual void Flush();
  virtual void Lock();
  virtual void Unlock();
  virtual void Clear(GUIColor &, bool overlay = false);
  virtual void ClearRect(GUIRect &);
  virtual void PushEvent(GUIEvent &event);

  static void ProcessEvent(picoTrackerEvent &event);
  static void ProcessButtonChange(uint16_t changeMask, uint16_t buttonMask);

  static picoTrackerGUIWindowImp *instance_;

protected:
  static chargfx_color_t GetColor(GUIColor &c);

  virtual void Update(Observable &o, I_ObservableData *d);

private:
  void SendFont(uint8_t uifontIndex);
  bool remoteUIEnabled_ = 0;
};
#endif
