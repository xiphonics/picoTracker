
#ifndef _SONG_VIEW_H_
#define _SONG_VIEW_H_

#include "ScreenView.h"

class SongView;

class SongView : public ScreenView {
public:
  SongView(GUIWindow &w, ViewData *viewData);
  ~SongView();

  // View implementation
  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int tick = 0);
  virtual void OnFocus();
  virtual void AnimationUpdate();

protected:
  void processNormalButtonMask(unsigned int mask);
  void processSelectionButtonMask(unsigned int mask);

  void extendSelection();
  void updateChain(int offset);
  void updateSongOffset(int offset);
  void updateCursor(int dx, int dy);
  void setChain(unsigned char);
  void cutPosition();
  void clonePosition();
  void pasteLast();
  void fillClipboardData();
  GUIRect getSelectionRect();
  void copySelection();
  void pasteClipboard();
  void cutSelection();

  void unMuteAll();
  void toggleMute();
  void switchSoloMode();

  void onStart();
  void startCurrentRow();
  void startImmediate();
  void onStop();

  void jumpToNextSection(int dir);

  void nudgeTempo(int direction);

private:
  bool updatingChain_; // .Flag that tells we're updating chain
                       //  so we don't allocate chains while
                       //  doing multiple A+ARROWS

  int updateX_; // . Position where update is happening
  int updateY_; //

  unsigned char lastChain_; // .Last chain clipboard

  int lastPlayedPosition_[SONG_CHANNEL_COUNT]; // .Last position played for song
                                               //  used for drawing purpose

  int lastQueuedPosition_[SONG_CHANNEL_COUNT]; // .Last live queued position for
                                               // song
                                               //  used for drawing purpose

  struct {                // .Clipboard structure
    bool active_;         // .If currently making a selection
    unsigned char *data_; // .Null if clipboard empty
    int x_;               // .Current selection positions
    int y_;               // .
    int offset_;          // .
    int width_;           // .Size of selection
    int height_;          // .
  } clipboard_;

  int saveX_;
  int saveY_;
  int saveOffset_;
  bool invertBatt_;
  bool needClear_;

  // Flags to track which UI elements need updating
  // These prevent core1 from directly updating the UI
  bool needsUIUpdate_ = false; // Single flag for all UI updates (notes, VU
                               // meter, positions, play time)
  bool needsPlayTimeUpdate_ = false; // Separate flag for play time updates

#ifdef PICO_DEOPTIMIZED_DEBUG
  // These variables are specifically for thread synchronization in debug builds
  // They create memory barriers between cores when manipulated in a specific
  // pattern DO NOT REMOVE - they are critical for performance in debug builds
  bool syncVar1_ = false;
  bool syncVar2_ = false;
  bool syncVar3_ = false;
#endif

  // Memory barrier function that uses the sync variables in debug DEOPTIMISED
  // builds only
  inline void createMemoryBarrier() {
#ifdef PICO_DEOPTIMIZED_DEBUG
    // This specific pattern of operations was found to be necessary
    // for proper thread synchronization in debug builds
    syncVar1_ = false;
    syncVar2_ = true;
    syncVar3_ = false;
#endif
  }
};

#endif
