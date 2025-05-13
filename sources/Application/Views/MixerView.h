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

protected:
  void processNormalButtonMask(unsigned int mask);
  void processSelectionButtonMask(unsigned int mask);
  void onStart();
  void onStop();
  void updateCursor(int dx, int dy);

  void unMuteAll();
  void toggleMute();
  void switchSoloMode();
  void togglePlay();

private:
  void drawChannelVUMeters(etl::array<stereosample, SONG_CHANNEL_COUNT> *levels,
                           Player *player, GUITextProperties props,
                           bool forceRedraw = false);
  void initChannelVolumeFields();

  // Channel volume UI fields
  etl::vector<UIIntVarField, SONG_CHANNEL_COUNT> channelVolumeFields_;
};
#endif