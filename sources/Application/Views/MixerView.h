#ifndef _MIXER_VIEW_H_
#define _MIXER_VIEW_H_

#include "FieldView.h"
#include "ViewData.h"
#include "BaseClasses/UIIntVarField.h"
#include <array>

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

private:
  void drawChannelVUMeters(etl::array<stereosample, SONG_CHANNEL_COUNT> *levels,
                           Player *player, GUITextProperties props);
  void initChannelVolumeFields();
  void updateChannelVolumeFields();
  void adjustChannelVolume(int channel, int delta);
  void resetChannelVolume(int channel);
  
  const char *song_;
  int saveX_;
  int saveY_;
  int saveOffset_;
  bool invertBatt_;
  
  // Channel volume UI fields
  std::array<UIIntVarField*, SONG_CHANNEL_COUNT> channelVolumeFields_;
  bool channelVolumeFieldsInitialized_;
};
#endif