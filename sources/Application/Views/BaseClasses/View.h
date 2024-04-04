
#ifndef _VIEW_H_
#define _VIEW_H_

#include "Application/Model/Config.h"
#include "Application/Model/Project.h"
#include "Application/Player/Player.h"
#include "Foundation/T_SimpleList.h"
#include "I_Action.h"
#include "UIFramework/Interfaces/I_GUIGraphics.h"
#include "UIFramework/SimpleBaseClasses/GUIWindow.h"
#include "ViewEvent.h"

enum GUIEventPadButtonMasks {
  EPBM_LEFT = 1,
  EPBM_DOWN = 2,
  EPBM_RIGHT = 4,
  EPBM_UP = 8,
  EPBM_L = 16,
  EPBM_B = 32,
  EPBM_A = 64,
  EPBM_R = 128,
  EPBM_START = 256,
  EPBM_SELECT = 512,
  EPBM_DOUBLE_A = 1024,
  EPBM_DOUBLE_B = 2048
};

enum ViewType {
  VT_SONG,
  VT_CHAIN,
  VT_PHRASE,
  VT_PROJECT,
  VT_INSTRUMENT,
  VT_TABLE,  // Table screen under phrase
  VT_TABLE2, // Table screen under instrument
  VT_GROOVE,
  VT_MIXER
};

enum ViewMode {
  VM_NORMAL,
  VM_NEW,
  VM_CLONE,
  VM_SELECTION,
  VM_MUTEON,
  VM_SOLOON
};

enum ColorDefinition {
  CD_BACKGROUND,
  CD_NORMAL,
  CD_HILITE1,
  CD_HILITE2,
  CD_CONSOLE,
  CD_CURSOR,
  CD_INFO,
  CD_WARN,
  CD_ERROR
};

enum ViewUpdateDirection { VUD_LEFT = 0, VUD_RIGHT, VUD_UP, VUD_DOWN };

class View;
class ModalView;

typedef void (*ModalViewCallback)(View &v, ModalView &d);

class View : public Observable {
public:
  View(GUIWindow &w, ViewData *viewData);
  View(View &v);

  void SetFocus(ViewType vt) {
    viewType_ = vt;
    hasFocus_ = true;
    OnFocus();
  };

  void LooseFocus() { hasFocus_ = false; };

  void Clear();

  void ProcessButton(unsigned short mask, bool pressed);

  void Redraw();

  // Override in subclasses

  virtual void DrawView() = 0;
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int currentTick) = 0;
  virtual void OnFocus() = 0;
  virtual void AnimationUpdate() = 0;

  void SetDirty(bool dirty);

  // Primitive locking mechanism

  bool Lock();
  void WaitForObject();
  void Unlock();

  // Char based draw routines

  virtual void SetColor(ColorDefinition cd);
  virtual void ClearRect(int x, int y, int w, int h);
  virtual void DrawString(int x, int y, const char *txt,
                          GUITextProperties &props);

  void DoModal(ModalView *view, ModalViewCallback cb = 0);

protected:
  virtual void ProcessButtonMask(unsigned short mask, bool pressed) = 0;

  // to remove once everything got to viewdata

  inline void updateData(unsigned char *c, int offset, unsigned char limit,
                         bool wrap) {
    int v = *c;
    if (v == 0xFF) { // Uninitiaized data
      v = 0;
    }
    v += offset;
    if (v < 0)
      v = (wrap ? (limit + 1 + v) : 0);
    if (v > limit)
      v = (wrap ? v - (limit + 1) : limit);
    *c = v;
  }

  GUIPoint GetAnchor();
  GUIPoint GetTitlePosition();

  void drawMap();
  void drawNotes();

  void drawBattery(float voltage, GUIPoint &pos, GUITextProperties &props);

  void drawMasterVuMeter(Player *player, GUIPoint pos, GUITextProperties props);

public: // temp hack for modl windo constructors
  GUIWindow &w_;
  ViewData *viewData_;

protected:
  ViewMode viewMode_;
  bool isDirty_; // .Do we need to redraw screeen
  ViewType viewType_;
  bool hasFocus_;

private:
  unsigned short mask_;
  bool locked_;
  static bool initPrivate_;
  ModalView *modalView_;
  ModalViewCallback modalViewCallback_;
  void setVUMeterColor_(int level);

public:
  static int margin_;
  static int songRowCount_;
  static bool miniLayout_;
};

#endif
