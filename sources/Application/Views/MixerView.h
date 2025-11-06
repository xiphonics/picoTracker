/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _MIXER_VIEW_H_
#define _MIXER_VIEW_H_

#include "BaseClasses/UIIntVarField.h"
#include "FieldView.h"
#include "Foundation/T_SimpleList.h"
#include "ViewData.h"

class MixerView : public FieldView {
public:
  MixerView(GUIWindow &w, ViewData *viewData);
  ~MixerView();
  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int tick = 0);
  virtual void OnFocus();
  virtual void AnimationUpdate();
  virtual void SetFocus(UIField *field); // Override to keep songX_ in sync

protected:
  void processNormalButtonMask(unsigned int mask);
  void processSelectionButtonMask(unsigned int mask);
  void onStart(){};
  void onStop(){};
  void updateCursor(int dx, int dy);

  void unMuteAll();
  void toggleMute();
  void switchSoloMode();
  void togglePlay();

private:
  void drawChannelVUMeters(etl::array<stereosample, SONG_CHANNEL_COUNT> *levels,
                           GUITextProperties props, bool forceRedraw = false);
  void initChannelVolumeFields();

  // Channel volume UI fields
  etl::vector<UIIntVarField, SONG_CHANNEL_COUNT> channelVolumeFields_;
  etl::vector<UIIntVarField, 1> masterVolumeField_; // Master volume field

  // Flags to track which UI elements need updating
  // These prevent core1 from directly updating the UI
  bool needsPlayTimeUpdate_ = false;
  bool needsNotesUpdate_ = false;
};
#endif
