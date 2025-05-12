
#ifndef _APP_WINDOW_H_
#define _APP_WINDOW_H_

#include "Application/Views/BaseClasses/View.h"
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
#define SCREEN_MAP_HEIGHT 4
#define SCREEN_MAP_WIDTH 4
#define BATTERY_GAUGE_WIDTH 5
#define SCREEN_CHARS SCREEN_WIDTH *SCREEN_HEIGHT
#define MAX_FIELD_WIDTH 26

// need this forward declaration to break out of circular dependency as
// ProjectView uses a UITextfield which in turn had dependency on AppWindow
// and UITextField is templated which means its class/method definitions need to
// be in its header file  :-(
class ProjectView;
class ChainView;
class ConsoleView;
class DeviceView;
class GrooveView;
class ImportView;
class InstrumentImportView;
class InstrumentView;
class NullView;
class PhraseView;
class SelectProjectView;
class SongView;
class TableView;
class ScreenView;
class MixerView;
class ThemeView;
class ThemeImportView;
class View;

class AppWindow : public GUIWindow, I_Observer, Status {
protected:
  AppWindow(I_GUIWindowImp &imp);
  virtual ~AppWindow();

public:
  static AppWindow *Create(GUICreateWindowParams &, const char *projectName);
  void LoadProject(const char *name);
  void CloseProject();

  virtual void Clear(bool all = false);
  virtual void ClearRect(GUIRect &rect);
  virtual void SetColor(ColorDefinition cd);
  void SetDirty();
  void UpdateColorsFromConfig();

  char projectName_[MAX_PROJECT_NAME_LENGTH];

protected: // GUIWindow implementation
  virtual bool onEvent(GUIEvent &event);
  virtual void onUpdate(bool redraw);
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

  void defineColor(FourCC colorCode, GUIColor &color, int paletteIndex);

  void onQuitApp();

private:
  bool autoSave();

  View *_currentView;
  ViewData *_viewData;
  SongView *_songView;
  ChainView *_chainView;
  PhraseView *_phraseView;
  DeviceView *_deviceView;
  ProjectView *_projectView;
  InstrumentView *_instrumentView;
  TableView *_tableView;
  GrooveView *_grooveView;
  ImportView *_importView;
  InstrumentImportView *_instrumentImportView;
  ThemeView *_themeView;
  ThemeImportView *_themeImportView;
  MixerView *_mixerView;
  SelectProjectView *_selectProjectView;
  NullView *_nullView;

  bool _isDirty;
  bool _closeProject;
  bool _shouldQuit;
  unsigned short _mask;
  unsigned long _lastA;
  unsigned long _lastB;
  char _statusLine[80];

  static unsigned char _charScreen[SCREEN_CHARS];
  static unsigned char _charScreenProp[SCREEN_CHARS];
  static unsigned char _preScreen[SCREEN_CHARS];
  static unsigned char _preScreenProp[SCREEN_CHARS];

  static GUIColor backgroundColor_;
  static GUIColor normalColor_;
  static GUIColor highlightColor_;
  static GUIColor highlight2Color_;
  static GUIColor consoleColor_;
  static GUIColor cursorColor_;
  static GUIColor infoColor_;
  static GUIColor warnColor_;
  static GUIColor errorColor_;
  static GUIColor accentColor_;
  static GUIColor accentAltColor_;
  static GUIColor emphasisColor_;
  static GUIColor reserved1Color_;
  static GUIColor reserved2Color_;
  static GUIColor reserved3Color_;
  static GUIColor reserved4Color_;

  ColorDefinition colorIndex_;

  static int charWidth_;
  static int charHeight_;

  SysMutex *drawMutex_;

  bool loadProject_ = false;

  uint32_t lastAutoSave = 0;
};

#endif
