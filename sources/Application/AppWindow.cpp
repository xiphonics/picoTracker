/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "AppWindow.h"
#include "Application/Commands/ApplicationCommandDispatcher.h"
#include "Application/Commands/EventDispatcher.h"
#include "Application/Instruments/SamplePool.h"
#include "Application/Mixer/MixerService.h"
#include "Application/Persistency/PersistencyService.h"
#include "Application/Player/TablePlayback.h"
#include "Application/Utils/char.h"
#include "Application/Views/ChainView.h"
#include "Application/Views/ConsoleView.h"
#include "Application/Views/DeviceView.h"
#include "Application/Views/GrooveView.h"
#include "Application/Views/ImportView.h"
#include "Application/Views/InstrumentImportView.h"
#include "Application/Views/InstrumentView.h"
#include "Application/Views/MixerView.h"
#include "Application/Views/ModalDialogs/FullScreenBox.h"
#include "Application/Views/ModalDialogs/MessageBox.h"
#include "Application/Views/NullView.h"
#include "Application/Views/PhraseView.h"
#include "Application/Views/ProjectView.h"
#include "Application/Views/RecordView.h"
#include "Application/Views/SampleEditorView.h"
#include "Application/Views/SelectProjectView.h"
#include "Application/Views/SongView.h"
#include "Application/Views/TableView.h"
#include "Application/Views/ThemeImportView.h"
#include "Application/Views/ThemeView.h"
#include "BaseClasses/View.h"
#include "Foundation/Variables/WatchedVariable.h"
#include "Player/Player.h"
#include "Services/Midi/MidiService.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/FileSystem.h"
#include "System/System/System.h"
#include "UIFramework/Interfaces/I_GUIWindowFactory.h"
#include "Views/UIController.h"
#include "platform.h"
#include <nanoprintf.h>
#include <string.h>

#ifdef ADV
#include "Adapters/adv/audio/record.h"
#else
#include "Adapters/picoTracker/audio/record.h"
#endif

const uint16_t AUTOSAVE_INTERVAL_IN_SECONDS = 1 * 60;

#define MINIMUM_ALLOWED_BATTERY_PERCENTAGE 2

#define MIN_BATT_POWEROFF_SEC 15

AppWindow *instance = 0;

unsigned char AppWindow::_charScreen[SCREEN_CHARS];
unsigned char AppWindow::_charScreenProp[SCREEN_CHARS];
unsigned char AppWindow::_preScreen[SCREEN_CHARS];
unsigned char AppWindow::_preScreenProp[SCREEN_CHARS];

GUIColor AppWindow::backgroundColor_(0x0F, 0x0F, 0x0F, 0);
GUIColor AppWindow::normalColor_(0xAD, 0xAD, 0xAD, 1);
GUIColor AppWindow::highlightColor_(0x84, 0x6F, 0x94, 2);
GUIColor AppWindow::highlight2Color_(0x6B, 0x31, 0x6B, 3);
GUIColor AppWindow::cursorColor_(0x77, 0x6B, 0x56, 4);
GUIColor AppWindow::consoleColor_(0xFF, 0x00, 0xFF, 5);
GUIColor AppWindow::infoColor_(0x29, 0xEE, 0x3D, 6);
GUIColor AppWindow::warnColor_(0xEF, 0xFA, 0x52, 7);
GUIColor AppWindow::errorColor_(0xE8, 0x4D, 0x15, 8);
GUIColor AppWindow::accentColor_(0x02, 0xFF, 0x02, 9);
GUIColor AppWindow::accentAltColor_(0xFF, 0x02, 0x02, 10);
GUIColor AppWindow::emphasisColor_(0xFF, 0xA5, 0x02, 11);
GUIColor AppWindow::reserved1Color_(0x02, 0x02, 0xFF, 12);
GUIColor AppWindow::reserved2Color_(0x55, 0x55, 0x55, 13);
GUIColor AppWindow::reserved3Color_(0x77, 0x77, 0x77, 14);
GUIColor AppWindow::reserved4Color_(0xFF, 0xFF, 0x00, 15);

// Initialize the animation frame counter
uint32_t AppWindow::animationFrameCounter_ = 0;

int AppWindow::charWidth_ = 8;
int AppWindow::charHeight_ = 8;

void AppWindow::defineColor(FourCC colorCode, GUIColor &color,
                            int paletteIndex) {

  Config *config = Config::GetInstance();
  auto rgbVar = config->FindVariable(colorCode);
  if (rgbVar) {
    const int32_t rgbValue = rgbVar->GetInt();
    unsigned short r, g, b;
    r = (rgbValue >> 16) & 0xFF;
    g = (rgbValue >> 8) & 0xFF;
    b = rgbValue & 0xFF;
    // Always preserve the palette index when updating colors
    color = GUIColor(r, g, b, paletteIndex);
  } else {
    // Even if we don't update the RGB values, ensure the palette index is
    // correct
    color._paletteIndex = paletteIndex;
  }
}

AppWindow::AppWindow(I_GUIWindowImp &imp) : GUIWindow(imp) {

  instance = this;

  // Init all members

  _statusLine[0] = 0;

  _currentView = 0;
  _viewData = 0;
  _songView = 0;
  _chainView = 0;
  _phraseView = 0;
  _deviceView = 0;
  _themeView = 0;
  _themeImportView = 0;
  _projectView = 0;
  _instrumentView = 0;
  _tableView = 0;
  _mixerView = 0;
  _sampleEditorView = 0;
  _recordView = 0;
  _nullView = 0;
  _grooveView = 0;
  _closeProject = 0;
  _lastA = 0;
  _lastB = 0;
  _mask = 0;
  lowBatteryMessageShown_ = false;
  lowBatteryWarningCounter_ = 0;

  EventDispatcher *ed = EventDispatcher::GetInstance();
  ed->SetWindow(this);

  Status::Install(this);

  // Init midi services
  MidiService::GetInstance()->Init();

  UpdateColorsFromConfig();

  GUIWindow::Clear(backgroundColor_);

  alignas(NullView) static char nullViewMemBuf[sizeof(NullView)];
  _nullView = new (nullViewMemBuf) NullView((*this), 0);
  _currentView = _nullView;
  _nullView->SetDirty(true);

  memset(_charScreen, ' ', SCREEN_CHARS);
  memset(_preScreen, ' ', SCREEN_CHARS);
  memset(_charScreenProp, 0, SCREEN_CHARS);
  memset(_preScreenProp, 0, SCREEN_CHARS);

  Redraw();

  // there is some sort of race that if we call LoadProject() from here directly
  // causes audio init to fail, so instead set this flag which will then cause
  // LoadProject() to be called from within the next time that AnimationUpdate()
  // is called
  loadProject_ = true;
};

AppWindow::~AppWindow() { MidiService::GetInstance()->Close(); }

void AppWindow::DrawString(const char *string, GUIPoint &pos,
                           GUITextProperties &props, bool force) {

  // Safety check for null string
  if (!string) {
    return;
  }

  // we know we don't have more than SCREEN_WIDTH chars
  char buffer[SCREEN_WIDTH + 1];
  int len = strlen(string);

  // Safety checks for offset calculation
  int offset = (pos._x < 0) ? -pos._x / 8 : 0;
  if (offset >= len) {
    return; // Nothing to draw if offset is beyond string length
  }

  len -= offset;
  int available = SCREEN_WIDTH - ((pos._x < 0) ? 0 : pos._x);
  len = std::min(len, available);

  // Additional safety check
  if (len <= 0) {
    return;
  }

  memcpy(buffer, string + offset, len);
  buffer[len] = 0;

  NAssert((pos._x < SCREEN_WIDTH) && (pos._y < SCREEN_HEIGHT));
  int index = pos._x + SCREEN_WIDTH * pos._y;
  memcpy(_charScreen + index, buffer, len);
  // Ensure color index is masked to prevent overlap with inversion bit
  unsigned char prop = (colorIndex_ & 0x7F) + (props.invert_ ? PROP_INVERT : 0);
  memset(_charScreenProp + index, prop, len);
};

void AppWindow::Clear(bool all) {
  memset(_charScreen, ' ', SCREEN_CHARS);
  memset(_charScreenProp, 0, SCREEN_CHARS);
  if (all) {
    memset(_preScreen, ' ', SCREEN_CHARS);
    memset(_preScreenProp, 0, SCREEN_CHARS);
  };
};

void AppWindow::ClearTextRect(GUIRect &r) {

  int x = r.Left();
  int y = r.Top();
  int w = r.Width();
  int h = r.Height();

  unsigned char *st = _charScreen + x + (SCREEN_WIDTH * y);
  unsigned char *pr = _charScreenProp + x + (SCREEN_WIDTH * y);
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      *st++ = ' ';
      *pr++ = 0;
    }
    st += (SCREEN_WIDTH - w);
    pr += (SCREEN_WIDTH - w);
  }
};

//
// Flush current screen to display
//
void AppWindow::Flush() {

  Lock();

  GUITextProperties props;
  GUIPoint pos;

  // Start with an invalid color to force color setting on first character
  ColorDefinition color = (ColorDefinition)-1;
  pos._x = 0;
  pos._y = 0;

  int count = 0;

  unsigned char *current = _charScreen;
  unsigned char *previous = _preScreen;
  unsigned char *currentProp = _charScreenProp;
  unsigned char *previousProp = _preScreenProp;
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
#ifndef _LGPT_NO_SCREEN_CACHE_
      if ((*current != *previous) || (*currentProp != *previousProp)) {
#endif
        // Extract invert flag from properties
        props.invert_ = (*currentProp & PROP_INVERT) != 0;

        // Extract color index from properties and check if it's different from
        // current color
        ColorDefinition charColor = (ColorDefinition)((*currentProp) & 0x7F);
        if (charColor != color) {
          color = charColor;

          // Initialize gcolor with a safe default to avoid uninitialized value
          // if switch falls through
          GUIColor gcolor = normalColor_;
          switch (color) {
          case CD_BACKGROUND:
            gcolor = backgroundColor_;
            break;
          case CD_NORMAL:
            break;
          case CD_HILITE1:
            gcolor = highlightColor_;
            break;
          case CD_HILITE2:
            gcolor = highlight2Color_;
            break;
          case CD_CONSOLE:
            gcolor = consoleColor_;
            break;
          case CD_CURSOR:
            gcolor = cursorColor_;
            break;
          case CD_INFO:
            gcolor = infoColor_;
            break;
          case CD_WARN:
            gcolor = warnColor_;
            break;
          case CD_ERROR:
            gcolor = errorColor_;
            break;
          case CD_ACCENT:
            gcolor = accentColor_;
            break;
          case CD_ACCENTALT:
            gcolor = accentAltColor_;
            break;
          case CD_EMPHASIS:
            gcolor = emphasisColor_;
            break;
          case CD_RESERVED1:
            gcolor = reserved1Color_;
            break;
          case CD_RESERVED2:
            gcolor = reserved2Color_;
            break;
          case CD_RESERVED3:
            gcolor = reserved3Color_;
            break;
          case CD_RESERVED4:
            gcolor = reserved4Color_;
            break;

          default:
            NAssert(0);
            break;
          }
          GUIWindow::SetColor(gcolor);
        }
        GUIWindow::DrawChar(*current, pos, props);
        count++;
#ifndef _LGPT_NO_SCREEN_CACHE_
      }
#endif
      current++;
      previous++;
      currentProp++;
      previousProp++;
      pos._x += AppWindow::charWidth_;
    }
    pos._y += AppWindow::charHeight_;
    pos._x = 0;
  }
  GUIWindow::Flush();
  Unlock();
  memcpy(_preScreen, _charScreen, SCREEN_CHARS);
  memcpy(_preScreenProp, _charScreenProp, SCREEN_CHARS);
};

AppWindow::LoadProjectResult AppWindow::LoadProject(const char *projectName) {

  _closeProject = false;

  PersistencyService *persist = PersistencyService::GetInstance();

  TablePlayback::Reset();

  // Load the sample pool
  SamplePool *pool = SamplePool::GetInstance();
  // load the projects samples
  pool->Load(projectName);

  alignas(Project) static char projectMemBuf[sizeof(Project)];
  Project *project = new (projectMemBuf) Project(projectName);

  bool succeeded = (persist->Load(projectName) == PERSIST_LOADED);
  if (!succeeded) {
    Trace::Error("Failed to load project '%s'", projectName);
    pool->Reset();
    TableHolder::GetInstance()->Reset();
    return LoadProjectResult::LOAD_FAILED;
  };

  // Project

  WatchedVariable::Disable();

  // Register as an observer of the project name variable to get notified of
  // changes
  Variable *projectNameVar = project->FindVariable(FourCC::VarProjectName);
  if (projectNameVar) {
    WatchedVariable *watchedVar = (WatchedVariable *)projectNameVar;
    if (watchedVar) {
      watchedVar->AddObserver(*this);
      // Store the initial project name
      project->GetProjectName(projectName_);
    }
  }

  project->GetInstrumentBank()->Init();

  WatchedVariable::Enable();

  ApplicationCommandDispatcher::GetInstance()->Init(project);

  // Create view data
  alignas(ViewData) static char viewDataMemBuf[sizeof(ViewData)];
  _viewData = new (viewDataMemBuf) ViewData(project);

  // Create & observe the player
  Player *player = Player::GetInstance();
  bool playerOK = player->Init(project, _viewData);
  player->AddObserver(*this);

  // Create the controller
  UIController *controller = UIController::GetInstance();
  controller->Init(project, _viewData);

  // Create & observe all views
  alignas(SongView) static char songViewMemBuf[sizeof(SongView)];
  _songView = new (songViewMemBuf) SongView((*this), _viewData);
  _songView->AddObserver((*this));

  alignas(ChainView) static char chainViewMemBuf[sizeof(ChainView)];
  _chainView = new (chainViewMemBuf) ChainView((*this), _viewData);
  _chainView->AddObserver((*this));

  alignas(PhraseView) static char phraseViewMemBuf[sizeof(PhraseView)];
  _phraseView = new (phraseViewMemBuf) PhraseView((*this), _viewData);
  _phraseView->AddObserver((*this));

  alignas(DeviceView) static char deviceViewMemBuf[sizeof(DeviceView)];
  _deviceView = new (deviceViewMemBuf) DeviceView((*this), _viewData);
  _deviceView->AddObserver((*this));

  alignas(ThemeView) static char themeViewMemBuf[sizeof(ThemeView)];
  _themeView = new (themeViewMemBuf) ThemeView((*this), _viewData);
  _themeView->AddObserver((*this));

  alignas(ThemeImportView) static char
      themeImportViewMemBuf[sizeof(ThemeImportView)];
  _themeImportView =
      new (themeImportViewMemBuf) ThemeImportView((*this), _viewData);
  _themeImportView->AddObserver((*this));

  alignas(ProjectView) static char projectViewMemBuf[sizeof(ProjectView)];
  _projectView = new (projectViewMemBuf) ProjectView((*this), _viewData);
  _projectView->AddObserver((*this));

  alignas(ImportView) static char importViewMemBuf[sizeof(ImportView)];
  _importView = new (importViewMemBuf) ImportView((*this), _viewData);
  _importView->AddObserver((*this));

  alignas(InstrumentImportView) static char
      instrumentImportViewMemBuf[sizeof(InstrumentImportView)];
  _instrumentImportView =
      new (instrumentImportViewMemBuf) InstrumentImportView((*this), _viewData);
  _instrumentImportView->AddObserver((*this));

  alignas(
      InstrumentView) static char instrumentViewMemBuf[sizeof(InstrumentView)];
  _instrumentView =
      new (instrumentViewMemBuf) InstrumentView((*this), _viewData);
  _instrumentView->AddObserver((*this));

  alignas(TableView) static char tableViewMemBuf[sizeof(TableView)];
  _tableView = new (tableViewMemBuf) TableView((*this), _viewData);
  _tableView->AddObserver((*this));

  alignas(GrooveView) static char grooveViewMemBuf[sizeof(GrooveView)];
  _grooveView = new (grooveViewMemBuf) GrooveView((*this), _viewData);
  _grooveView->AddObserver(*this);

  alignas(SelectProjectView) static char
      selectProjectViewMemBuf[sizeof(SelectProjectView)];
  _selectProjectView =
      new (selectProjectViewMemBuf) SelectProjectView((*this), _viewData);
  _selectProjectView->AddObserver((*this));

  alignas(MixerView) static char mixerViewMemBuf[sizeof(MixerView)];
  _mixerView = new (mixerViewMemBuf) MixerView((*this), _viewData);
  _mixerView->AddObserver((*this));

  alignas(SampleEditorView) static char
      sampleEditorViewMemBuf[sizeof(SampleEditorView)];
  _sampleEditorView =
      new (sampleEditorViewMemBuf) SampleEditorView((*this), _viewData);
  _sampleEditorView->AddObserver((*this));

  alignas(RecordView) static char recordViewMemBuf[sizeof(RecordView)];
  _recordView = new (recordViewMemBuf) RecordView((*this), _viewData);
  _recordView->AddObserver((*this));

  _currentView = _songView;
  _currentView->OnFocus();

  if (!playerOK) {
    MessageBox *mb =
        MessageBox::Create(*_songView, "Failed to initialize audio", MBBF_OK);
    _songView->DoModal(mb);
  }

  if (_currentView) {
    _currentView->SetDirty(true);
    SetDirty();
  }
  return LoadProjectResult::LOAD_OK;
}

void AppWindow::CloseProject() {

  _closeProject = false;
  Player *player = Player::GetInstance();
  player->Stop();
  player->RemoveObserver(*this);

  player->Reset();

  SamplePool *pool = SamplePool::GetInstance();
  pool->Reset();

  TableHolder::GetInstance()->Reset();
  TablePlayback::Reset();

  ApplicationCommandDispatcher::GetInstance()->Close();

  if (_songView) {
    _songView->~SongView();
    _songView = nullptr;
  }
  if (_chainView) {
    _chainView->~ChainView();
    _chainView = nullptr;
  }
  if (_phraseView) {
    _phraseView->~PhraseView();
    _phraseView = nullptr;
  }
  if (_deviceView) {
    _deviceView->~DeviceView();
    _deviceView = nullptr;
  }
  if (_themeView) {
    _themeView->~ThemeView();
    _themeView = nullptr;
  }
  if (_themeImportView) {
    _themeImportView->~ThemeImportView();
    _themeImportView = nullptr;
  }
  if (_projectView) {
    _projectView->~ProjectView();
    _projectView = nullptr;
  }
  if (_instrumentView) {
    _instrumentView->~InstrumentView();
    _instrumentView = nullptr;
  }
  if (_tableView) {
    _tableView->~TableView();
    _tableView = nullptr;
  }
  if (_grooveView) {
    _grooveView->~GrooveView();
    _grooveView = nullptr;
  }

  UIController *controller = UIController::GetInstance();
  controller->Reset();

  if (_viewData) {
    _viewData->~ViewData();
    _viewData = nullptr;
  }

  _currentView = _nullView;
  _nullView->SetDirty(true);
};

AppWindow *AppWindow::Create(GUICreateWindowParams &params,
                             const char *projectName) {
  I_GUIWindowImp &imp =
      I_GUIWindowFactory::GetInstance()->CreateWindowImp(params);
  alignas(AppWindow) static char appWindowMemBuf[sizeof(AppWindow)];
  AppWindow *w = new (appWindowMemBuf) AppWindow(imp);
  strcpy(w->projectName_, projectName);
  return w;
};

void AppWindow::SetDirty() {
  if (_currentView) {
    _currentView->SetDirty(true);
  }
};

void AppWindow::UpdateColorsFromConfig() {
  // now assign custom colors if they have been set device config
  defineColor(FourCC::VarBGColor, backgroundColor_, 0);
  defineColor(FourCC::VarFGColor, normalColor_, 1);
  defineColor(FourCC::VarHI1Color, highlightColor_, 2);
  defineColor(FourCC::VarHI2Color, highlight2Color_, 3);
  defineColor(FourCC::VarCursorColor, cursorColor_, 4);
  defineColor(FourCC::VarConsoleColor, consoleColor_, 5);
  defineColor(FourCC::VarInfoColor, infoColor_, 6);
  defineColor(FourCC::VarWarnColor, warnColor_, 7);
  defineColor(FourCC::VarErrorColor, errorColor_, 8);
  defineColor(FourCC::VarAccentColor, accentColor_, 9);
  defineColor(FourCC::VarAccentAltColor, accentAltColor_, 10);
  defineColor(FourCC::VarEmphasisColor, emphasisColor_, 11);

  // These are commented out so they are not included in config or theme exports
  // until they are actually used in the future
  // defineColor(FourCC::VarReserved1Color, reserved1Color_, 12);
  // defineColor(FourCC::VarReserved2Color, reserved2Color_, 13);
  // defineColor(FourCC::VarReserved3Color, reserved3Color_, 14);
  // defineColor(FourCC::VarReserved4Color, reserved4Color_, 15);
};

bool AppWindow::onEvent(GUIEvent &event) {

  // We need to tell the app to quit once we're out of the
  // mixer lock, otherwise the windows driver will never return

  _shouldQuit = false;

  unsigned short v = 1 << event.GetValue();

  MixerService *sm = MixerService::GetInstance();
  // TODO(democloid): this causes a deadlock, verify original intent
  //  MixerService *ms = MixerService::GetInstance();
  //  ms->Lock();

  switch (event.GetType()) {

  case ET_PADBUTTONDOWN:

    _mask |= v;
    if (_currentView)
      _currentView->ProcessButton(_mask, true);
    break;

  case ET_PADBUTTONUP:

    _mask &= (0xFFFF - v);
    if (_currentView)
      _currentView->ProcessButton(_mask, false);
    break;

  case ET_SYSQUIT:
    _shouldQuit = true;
    break;

    /*		case ET_KEYDOWN:
            if
       (event.GetValue()==EKT_ESCAPE&&!Player::GetInstance()->IsRunning()) {
       if
       (_currentView!=_listView) {
       CloseProject() ;
       _currentView->SetDirty(true) ;
       } else {
                            System::GetInstance()->PostQuitMessage() ;
                    };
            } ;
                */

  default:
    break;
  }
  //  ms->Unlock();

  if (_shouldQuit) {
    onQuitApp();
  }
  if (_closeProject) {
    CloseProject();
    SetDirty();
  }

  // View dirty flag will be checked in AnimationUpdate to determine if redraw
  // is needed
  return false;
};

void AppWindow::onUpdate(bool redraw) {
  if (redraw) {
    GUIWindow::Clear(backgroundColor_, true);
    Clear(true);
    // Mark as dirty to trigger redraw in AnimationUpdate
    SetDirty();
  }
  // No Flush here - AnimationUpdate will handle it
};

void AppWindow::AnimationUpdate() {
  // Increment the animation frame counter
  animationFrameCounter_++;
  char failedProjectName_[MAX_PROJECT_NAME_LENGTH] = {0};

  if (awaitingProjectLoadAck_) {
    if (_mask != 0) {
      FileSystem::GetInstance()->DeleteFile("/.current");
      npf_snprintf(projectName_, sizeof(projectName_), "%s",
                   UNNAMED_PROJECT_NAME);
      loadProject_ = true;
      awaitingProjectLoadAck_ = false;
      Trace::Error("Falling back to untitled after failed load of '%s'",
                   failedProjectName_);
    }
    return;
  }

  if (loadProject_) {
    LoadProjectResult loadResult = LoadProject(projectName_);
    loadProject_ = false;
    if (loadResult == LoadProjectResult::LOAD_FAILED) {
      npf_snprintf(failedProjectName_, sizeof(failedProjectName_), "%s",
                   projectName_);
      Status::SetMultiLine(
          "Invalid Project:\n%s\n  \nPress any key\nto continue...",
          failedProjectName_);
      Trace::Error(
          "Failed to load project '%s'. Waiting for key press to load untitled",
          failedProjectName_);
      awaitingProjectLoadAck_ = true;
      return;
    }
  }

  // run at 1Hz
  if (animationFrameCounter_ % PICO_CLOCK_HZ == 0) {
    // Check battery level
    BatteryState batteryState;
    System::GetInstance()->GetBatteryState(batteryState);
    if (!batteryState.error) {
      // only process battery status if no error state
      if (batteryState.percentage < MINIMUM_ALLOWED_BATTERY_PERCENTAGE &&
          !batteryState.charging) {
        lowBatteryState_ = true;
        lowBatteryWarningCounter_++;
      } else {
        lowBatteryState_ = false;
        lowBatteryWarningCounter_ = 0;
      }
      if (lowBatteryWarningCounter_ > MIN_BATT_POWEROFF_SEC) {
        System::GetInstance()->PowerDown();
      }
    }
  }

  if (lowBatteryState_ && !lowBatteryMessageShown_) {
    if (!_currentView->HasModalView()) {
      FullScreenBox *mb = FullScreenBox::Create(*_currentView, "Low battery!",
                                                "Connect charger", 0);
      _currentView->DoModal(mb);
      lowBatteryMessageShown_ = true;
      SetDirty();
    }
  } else if (!lowBatteryState_ && lowBatteryMessageShown_) {
    ModalView *modal = _currentView->GetModalView();
    if (modal) {
      modal->EndModal(0);
      _currentView->DismissModal();
      Trace::Debug("CLose Low Batt dialog");
    }
    lowBatteryMessageShown_ = false;
    SetDirty();
  }

  // If we need a full redraw due to state changes from key events
  if (_currentView && _currentView->isDirty()) {
    _currentView->Redraw(); // Draw main content
  }

  // Handle view updates
  if (_currentView) {
    // Always update the main view even if modal is active because things like
    // batt gauge still need redrawing and visibility even with a modal onscreen
    _currentView->AnimationUpdate();
    // Now check if there's an active modal view and
    ModalView *modalView = _currentView->GetModalView();
    if (modalView) {
      // Update the modal view
      modalView->AnimationUpdate();
    }
  }

  // Always flush after AnimationUpdate to ensure consistent state
  Flush();

  // *attempt* to auto save every AUTOSAVE_INTERVAL_IN_SECONDS
  // will return false if auto save was unsuccessful because eg. the sequencer
  // is running
  // we do this here because for sheer convenience because this
  // this callback is called PICO_CLOCK_HZ times a second and we have easy
  // access in this class to the player, projectname and persistence service
  if ((++lastAutoSave / PICO_CLOCK_HZ) > AUTOSAVE_INTERVAL_IN_SECONDS) {
    if (autoSave()) {
      lastAutoSave = 0;
    }
  }
}

void AppWindow::LayoutChildren(){};

void AppWindow::Update(Observable &o, I_ObservableData *d) {
  if (d && (uintptr_t)d == (uintptr_t)FourCC::VarProjectName) {
    // Update the stored project name from the project
    Project *project = _viewData->project_;
    if (project) {
      project->GetProjectName(projectName_);
      Trace::Log("APPWINDOW", "Project name retrieved: %s", projectName_);
    } else {
      Trace::Error("APPWINDOW: Project name retrieval failed!");
    }
    return;
  }

  ViewEvent *ve = (ViewEvent *)d;

  switch (ve->GetType()) {

  case VET_SWITCH_VIEW: {
    ViewType *vt = (ViewType *)ve->GetData();
    if (_currentView) {
      _currentView->LooseFocus();
    }

    switch (*vt) {
    case VT_SONG:
      _currentView = _songView;
      break;
    case VT_CHAIN:
      _currentView = _chainView;
      break;
    case VT_PHRASE:
      _currentView = _phraseView;
      break;
    case VT_DEVICE:
      _currentView = _deviceView;
      break;
    case VT_PROJECT:
      _currentView = _projectView;
      break;
    case VT_INSTRUMENT:
      _currentView = _instrumentView;
      break;
    case VT_TABLE:
      _currentView = _tableView;
      break;
    case VT_TABLE2:
      _currentView = _tableView;
      break;
    case VT_GROOVE:
      _currentView = _grooveView;
      break;
    case VT_IMPORT:
      _currentView = _importView;
      break;
    case VT_INSTRUMENT_IMPORT:
      _currentView = _instrumentImportView;
      break;
    case VT_SELECTPROJECT:
      _currentView = _selectProjectView;
      break;
    case VT_MIXER:
      _currentView = _mixerView;
      break;
    case VT_THEME:
      _currentView = _themeView;
      break;
    case VT_THEME_IMPORT:
      _currentView = _themeImportView;
      break;
    case VT_SELECTTHEME:
      _currentView = _themeView;
      break;
    case VT_SAMPLE_EDITOR:
      _currentView = _sampleEditorView;
      break;
    case VT_RECORD:
      _currentView = _recordView;
      break;
    default:
      break;
    }
    _currentView->SetFocus(*vt);
    SetDirty();
    GUIWindow::Clear(backgroundColor_, true);
    Clear(true);
    break;
  }

  case VET_PLAYER_POSITION_UPDATE: {
    PlayerEvent *pt = (PlayerEvent *)ve;
    if (_currentView) {
      // Check if the current view has a modal view
      if (_currentView->HasModalView()) {
        _currentView->GetModalView()->OnPlayerUpdate(pt->GetType(),
                                                     pt->GetTickCount());
      } else {
        _currentView->OnPlayerUpdate(pt->GetType(), pt->GetTickCount());
      }
      Invalidate();
    }
    break;
  }

    /*	  case VET_LIST_SELECT:
          {
          char *name=(char*)ve->GetData() ;
          LoadProject(name) ;
          break ;
          } */
  case VET_QUIT_PROJECT: {
    // defer event to after we got out of the view
    _closeProject = true;
    break;
  }
  case VET_QUIT_APP:
    _shouldQuit = true;
    break;
  default: // VET_LIST_SELECT, VET_UPDATE
    break;
  }
}

void AppWindow::onQuitApp() {
  Player *player = Player::GetInstance();
  player->Stop();
  player->RemoveObserver(*this);

  player->Reset();
  System::GetInstance()->PostQuitMessage();
}

void AppWindow::Print(char *line) { PrintMultiLine(line); }

void AppWindow::PrintMultiLine(char *line) {
  Clear();
  GUITextProperties props;
  SetColor(CD_NORMAL);

  int current_y = 11; // Start near the middle of the screen

  // Handle single line case for Print function
  bool isSingleLine = (line != nullptr && strchr(line, '\n') == nullptr);

  // Use strtok to split the string by newline characters
  char *token = strtok(line, "\n");
  int lineCount = 0;

  while (token != NULL) {
    // Stop if we are about to overwrite the build string line
    if (current_y >= 22) {
      break;
    }

    // For single line, center it at position 12 instead of starting at 11
    if (isSingleLine && lineCount == 0) {
      current_y = 12;
    }

    // Horizontally center the current line of text
    int position = 32; // Assumes a screen width of 32 characters
    position -= strlen(token);
    position /= 2;

    GUIPoint pos(position, current_y);
    DrawString(token, pos, props);

    // Get the next line
    token = strtok(NULL, "\n");
    current_y++;
    lineCount++;
  }

  // Preserve the build string at the bottom of the screen
  char buildString[SCREEN_WIDTH + 1];
  npf_snprintf(buildString, sizeof(buildString), "picoTracker build %s%s_%s",
               PROJECT_NUMBER, PROJECT_RELEASE, BUILD_COUNT);
  GUIPoint pos(0, 22);
  pos._x = (32 - strlen(buildString)) / 2;
  DrawString(buildString, pos, props);
  Flush();
};

void AppWindow::SetColor(ColorDefinition cd) {
  // Ensure color index is within valid range (0-15)
  if (cd <= CD_EMPHASIS) {
    colorIndex_ = cd;
  } else {
    Trace::Error("APPWINDOW", "Invalid color index: %d", cd);
    colorIndex_ = CD_NORMAL; // Default to normal color
  }
};

bool AppWindow::autoSave() {
  Player *player = Player::GetInstance();
  // only auto save when sequencer is not running and not recording
  bool recording = IsRecordingActive();
  if (!player->IsRunning() && !recording) {
    Trace::Log("APPWINDOW", "AutoSaving Project Data");
    // get persistence service and call autosave
    PersistencyService *ps = PersistencyService::GetInstance();
    auto result = ps->AutoSaveProjectData(projectName_);
    if (result != PERSIST_SAVED) {
      Trace::Error("APPWINDOW", "Failed to auto-save project data");
      // we dont return false here as we dont want to go into a bombardment of
      // auto save attempts and instead just attempt to auto save again after
      // the next interval
    }
    return true;
  }
  return false;
}

// Maps the ColorDefinition used in View classes to a GUIColor that is needed by
// DrawRect which unlike the text grid based drawing code works in terms of
// GUIColor and not definitions
GUIColor AppWindow::GetColor(ColorDefinition cd) {
  switch (cd) {
  case CD_NORMAL:
    return AppWindow::normalColor_;
  case CD_BACKGROUND:
    return AppWindow::backgroundColor_;
  case CD_HILITE1:
    return AppWindow::highlightColor_;
  case CD_HILITE2:
    return AppWindow::highlight2Color_;
  case CD_CONSOLE:
    return AppWindow::consoleColor_;
  case CD_CURSOR:
    return AppWindow::cursorColor_;
  case CD_INFO:
    return AppWindow::infoColor_;
  case CD_WARN:
    return AppWindow::warnColor_;
  case CD_ERROR:
    return AppWindow::errorColor_;
  case CD_ACCENT:
    return AppWindow::accentColor_;
  case CD_ACCENTALT:
    return AppWindow::accentAltColor_;
  case CD_EMPHASIS:
    return AppWindow::emphasisColor_;
  case CD_RESERVED1:
    return AppWindow::reserved1Color_;
  case CD_RESERVED2:
    return AppWindow::reserved2Color_;
  case CD_RESERVED3:
    return AppWindow::reserved3Color_;
  case CD_RESERVED4:
    return AppWindow::reserved4Color_;

  default:
    break;
  }
  return AppWindow::normalColor_;
}
