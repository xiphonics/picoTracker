
#ifndef _APP_WINDOW_H_
#define _APP_WINDOW_H_

#include "Application/Views/ChainView.h"
#include "Application/Views/ConsoleView.h"
#include "Application/Views/GrooveView.h"
#include "Application/Views/InstrumentView.h"
#include "Application/Views/NullView.h"
#include "Application/Views/PhraseView.h"
#include "Application/Views/ProjectView.h"
#include "Application/Views/SongView.h"
#include "Application/Views/TableView.h"
#include "Application/Views/ViewData.h"
#include "Foundation/Observable.h"
#include "System/Process/SysMutex.h"
#include "System/io/Status.h"
#include "UIFramework/SimpleBaseClasses/GUIWindow.h"

#define PROP_INVERT 0x80
#define CHAR_WIDTH 10
#define CHAR_HEIGHT 10
#define SCREEN_WIDTH 32
#define SCREEN_HEIGHT 24
#define SCREEN_CHARS SCREEN_WIDTH *SCREEN_HEIGHT

class AppWindow : public GUIWindow, I_Observer, Status {
protected:
  AppWindow(I_GUIWindowImp &imp);
  virtual ~AppWindow();

public:
  static AppWindow *Create(GUICreateWindowParams &);
  void LoadProject(const Path &path);
  void CloseProject();

  virtual void Clear(bool all = false);
  virtual void ClearRect(GUIRect &rect);
  virtual void SetColor(ColorDefinition cd);
  void SetDirty();

protected: // GUIWindow implementation
  virtual bool onEvent(GUIEvent &event);
  virtual void onUpdate();
  virtual void LayoutChildren();
  virtual void Flush();
  virtual void Redraw();
  virtual void AnimationUpdate();

  // override draw string to avoid going too far off
  // the screen.
  virtual void DrawString(const char *string, GUIPoint &pos,
                          GUITextProperties &props, bool overlay = false);

  // I_Observer implementation

  virtual void Update(Observable &o, I_ObservableData *d);

  // Status implementation

  virtual void Print(char *);

  void defineColor(const char *colorName, GUIColor &color, int paletteIndex);

  void onQuitApp();

private:
  View *_currentView;
  ViewData *_viewData;
  SongView *_songView;
  ChainView *_chainView;
  PhraseView *_phraseView;
  ProjectView *_projectView;
  InstrumentView *_instrumentView;
  TableView *_tableView;
  GrooveView *_grooveView;
  NullView *_nullView;

  Path _root;

  bool _isDirty;
  bool _closeProject;
  bool _loadAfterSaveAsProject;
  bool _shouldQuit;
  unsigned short _mask;
  unsigned long _lastA;
  unsigned long _lastB;
  char _statusLine[80];
  char _newProjectToLoad[80];

  static unsigned char _charScreen[SCREEN_CHARS];
  static unsigned char _charScreenProp[SCREEN_CHARS];
  static unsigned char _preScreen[SCREEN_CHARS];
  static unsigned char _preScreenProp[SCREEN_CHARS];

  static GUIColor backgroundColor_;
  static GUIColor normalColor_;
  static GUIColor highlight2Color_;
  static GUIColor highlightColor_;
  static GUIColor consoleColor_;
  static GUIColor cursorColor_;
  static GUIColor infoColor_;
  static GUIColor warnColor_;
  static GUIColor errorColor_;

  ColorDefinition colorIndex_;

  static int charWidth_;
  static int charHeight_;

  SysMutex drawMutex_;
};

#endif
