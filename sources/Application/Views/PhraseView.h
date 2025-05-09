
#ifndef _PHRASE_VIEW_H_
#define _PHRASE_VIEW_H_

#include "BaseClasses/UIBigHexVarField.h"
#include "ScreenView.h"
#include "ViewData.h"

class PhraseView : public ScreenView {

public:
  PhraseView(GUIWindow &w, ViewData *viewData);
  ~PhraseView();
  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int tick = 0);
  virtual void OnFocus();
  virtual void AnimationUpdate();

protected:
  void updateCursor(int dx, int dy);
  void updateCursorValue(ViewUpdateDirection offset, int xOffset = 0,
                         int yOffset = 0);
  void updateSelectionValue(ViewUpdateDirection direction);
  void warpToNeighbour(int offset);
  void warpInChain(int offset);
  void cutPosition();
  void pasteLast();

  void extendSelection();

  GUIRect getSelectionRect();
  void fillClipboardData();
  void copySelection();
  void cutSelection();
  void pasteClipboard();

  void startAudition(bool startIfNotRunning);
  void stopAudition();
  void unMuteAll();
  void toggleMute();
  void switchSoloMode();

  void processNormalButtonMask(unsigned short mask);
  void processSelectionButtonMask(unsigned short mask);

  void setTextProps(GUITextProperties &props, int row, int col, bool restore);

private:
  int row_;
  int col_;
  int lastNote_;
  int lastInstr_;
  int lastCmd_;
  int lastParam_;
  Phrase *phrase_;
  int lastPlayingPos_;
  Variable cmdEdit_;
  UIBigHexVarField *cmdEditField_;
  void printHelpLegend(FourCC command, GUITextProperties props);

  struct clipboard {
    bool active_;
    int col_;
    int row_;
    int width_;
    int height_;
    uchar note_[16];
    uchar instr_[16];
    uchar cmd1_[16];
    ushort param1_[16];
    uchar cmd2_[16];
    ushort param2_[16];
  } clipboard_;

  int saveCol_;
  int saveRow_;

  static short offsets_[2][4];

  // Flags to track which UI elements need updating
  // These prevent core1 from directly updating the UI
  bool needsUIUpdate_ = false; // Single flag for all UI updates (notes, VU meter, positions, live indicators)
  
  // Keep these for backward compatibility
  bool needsPlayPositionUpdate_ = false;
  bool needsLiveIndicatorUpdate_ = false;
  bool needsNotesUpdate_ = false;
  bool needsVUMeterUpdate_ = false;

#ifdef PICO_DEOPTIMIZED_DEBUG
  // These variables are specifically for thread synchronization in debug builds
  // They create memory barriers between cores when manipulated in a specific pattern
  // DO NOT REMOVE - they are critical for performance in debug builds
  bool syncVar1_ = false;
  bool syncVar2_ = false;
  bool syncVar3_ = false;
#endif

  // Memory barrier function that uses the sync variables in debug DEOPTIMISED builds only
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
