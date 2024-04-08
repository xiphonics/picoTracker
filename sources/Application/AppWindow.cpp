#include "AppWindow.h"
#include "Application/Commands/ApplicationCommandDispatcher.h"
#include "Application/Commands/EventDispatcher.h"
#include "Application/Instruments/SamplePool.h"
#include "Application/Mixer/MixerService.h"
#include "Application/Persistency/PersistencyService.h"
#include "Application/Player/TablePlayback.h"
#include "Application/Utils/char.h"
#include "Application/Views/ModalDialogs/MessageBox.h"
#include "Application/Views/ModalDialogs/SelectProjectDialog.h"
#include "Foundation/Variables/WatchedVariable.h"
#include "Player/Player.h"
#include "Services/Midi/MidiService.h"
#include "System/Console/Trace.h"
#include "UIFramework/Interfaces/I_GUIWindowFactory.h"
#include "Views/UIController.h"
#include <string.h>

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

int AppWindow::charWidth_ = 8;
int AppWindow::charHeight_ = 8;

// #define _FORCE_SDL_EVENT_

static void ProjectSelectCallback(View &v, ModalView &dialog) {

  SelectProjectDialog &spd = (SelectProjectDialog &)dialog;
  if (dialog.GetReturnCode() > 0) {
    Path selected = spd.GetSelection();
    instance->LoadProject(selected.GetPath().c_str());
  } else {
    System::GetInstance()->PostQuitMessage();
  }
};

void AppWindow::defineColor(const char *colorName, GUIColor &color,
                            int paletteIndex) {

  Config *config = Config::GetInstance();
  const char *value = config->GetValue(colorName);
  if (value) {
    unsigned char r;
    char2hex(value, &r);
    unsigned char g;
    char2hex(value + 2, &g);
    unsigned char b;
    char2hex(value + 4, &b);
    color = GUIColor(r, g, b, paletteIndex);
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
  _projectView = 0;
  _instrumentView = 0;
  _tableView = 0;
  _nullView = 0;
  _grooveView = 0;
  _closeProject = 0;
  _lastA = 0;
  _lastB = 0;
  _mask = 0;
  colorIndex_ = CD_NORMAL;

  EventDispatcher *ed = EventDispatcher::GetInstance();
  ed->SetWindow(this);

  Status::Install(this);

  // Init midi services
  MidiService::GetInstance()->Init();

  // now assign custom colors if they have been set in the config.xml
  defineColor("BACKGROUND", backgroundColor_, 0);
  defineColor("FOREGROUND", normalColor_, 1);
  cursorColor_ = normalColor_;
  defineColor("HICOLOR1", highlightColor_, 2);
  defineColor("HICOLOR2", highlight2Color_, 3);
  defineColor("CURSORCOLOR", cursorColor_, 4);
  defineColor("INFOCOLOR", infoColor_, 5);
  defineColor("WARNCOLOR", warnColor_, 6);
  defineColor("ERRORCOLOR", errorColor_, 7);

  GUIWindow::Clear(backgroundColor_);

  _nullView = new NullView((*this), 0);
  _currentView = _nullView;
  _nullView->SetDirty(true);

  SelectProjectDialog *spd = new SelectProjectDialog(*_currentView);
  _currentView->DoModal(spd, ProjectSelectCallback);

  memset(_charScreen, ' ', SCREEN_CHARS);
  memset(_preScreen, ' ', SCREEN_CHARS);
  memset(_charScreenProp, 0, SCREEN_CHARS);
  memset(_preScreenProp, 0, SCREEN_CHARS);

  Redraw();
};

AppWindow::~AppWindow() { MidiService::GetInstance()->Close(); }

void AppWindow::DrawString(const char *string, GUIPoint &pos,
                           GUITextProperties &props, bool force) {

  // we know we don't have more than SCREEN_WIDTH chars

  char buffer[SCREEN_WIDTH + 1];
  int len = strlen(string);
  int offset = (pos._x < 0) ? -pos._x / 8 : 0;
  len -= offset;
  int available = SCREEN_WIDTH - ((pos._x < 0) ? 0 : pos._x);
  len = MIN(len, available);
  memcpy(buffer, string + offset, len);
  buffer[len] = 0;

  NAssert((pos._x < SCREEN_WIDTH) && (pos._y < SCREEN_HEIGHT));
  int index = pos._x + SCREEN_WIDTH * pos._y;
  memcpy(_charScreen + index, buffer, len);
  unsigned char prop = colorIndex_ + (props.invert_ ? PROP_INVERT : 0);
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

void AppWindow::ClearRect(GUIRect &r) {

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
// Redraws the screen and flush it.
//

void AppWindow::Redraw() {

  SysMutexLocker locker(drawMutex_);

  if (_currentView) {
    _currentView->Redraw();
    Invalidate();
  }
};

//
// Flush current screen to display
//

void AppWindow::Flush() {

  SysMutexLocker locker(drawMutex_);

  Lock();

  GUITextProperties props;
  GUIPoint pos;

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
        props.invert_ = (*currentProp & PROP_INVERT) != 0;
        if (((*currentProp) & 0x7F) != color) {
          color = (ColorDefinition)((*currentProp) & 0x7F);
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

void AppWindow::LoadProject(const Path &p) {

  _root = p;

  _closeProject = false;

  PersistencyService *persist = PersistencyService::GetInstance();

  TablePlayback::Reset();

  Path::SetAlias("project", _root.GetPath().c_str());
  Path::SetAlias("samples", "project:samples");

  // Load the sample pool

  SamplePool *pool = SamplePool::GetInstance();

  pool->Load();

  Project *project = new Project();

  bool succeeded = persist->Load();
  if (!succeeded) {
    project->GetInstrumentBank()->AssignDefaults();
  };

  // Project

  WatchedVariable::Disable();

  project->GetInstrumentBank()->Init();

  WatchedVariable::Enable();

  ApplicationCommandDispatcher::GetInstance()->Init(project);

  // Create view data

  _viewData = new ViewData(project);

  // Create & observe the player
  Player *player = Player::GetInstance();
  bool playerOK = player->Init(project, _viewData);
  player->AddObserver(*this);

  // Create the controller
  UIController *controller = UIController::GetInstance();
  controller->Init(project, _viewData);

  // Create & observe all views
  _songView = new SongView((*this), _viewData, _root.GetName().c_str());
  _songView->AddObserver((*this));

  _chainView = new ChainView((*this), _viewData);
  _chainView->AddObserver((*this));

  _phraseView = new PhraseView((*this), _viewData);
  _phraseView->AddObserver((*this));

  _projectView = new ProjectView((*this), _viewData);
  _projectView->AddObserver((*this));

  _instrumentView = new InstrumentView((*this), _viewData);
  _instrumentView->AddObserver((*this));

  _tableView = new TableView((*this), _viewData);
  _tableView->AddObserver((*this));

  _grooveView = new GrooveView((*this), _viewData);
  _grooveView->AddObserver(*this);

  _currentView = _songView;
  _currentView->OnFocus();

  if (!playerOK) {
    MessageBox *mb =
        new MessageBox(*_songView, "Failed to initialize audio", MBBF_OK);
    _songView->DoModal(mb);
  }

  Redraw();
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

  SAFE_DELETE(_songView);
  SAFE_DELETE(_chainView);
  SAFE_DELETE(_phraseView);
  SAFE_DELETE(_projectView);
  SAFE_DELETE(_instrumentView);
  SAFE_DELETE(_tableView);
  SAFE_DELETE(_grooveView);

  UIController *controller = UIController::GetInstance();
  controller->Reset();

  SAFE_DELETE(_viewData);

  _currentView = _nullView;
  _nullView->SetDirty(true);

  SelectProjectDialog *spd = new SelectProjectDialog(*_currentView);
  _currentView->DoModal(spd, ProjectSelectCallback);
};

AppWindow *AppWindow::Create(GUICreateWindowParams &params) {
  I_GUIWindowImp &imp =
      I_GUIWindowFactory::GetInstance()->CreateWindowImp(params);
  AppWindow *w = new AppWindow(imp);
  return w;
};

void AppWindow::SetDirty() { _isDirty = true; };

bool AppWindow::onEvent(GUIEvent &event) {

  // We need to tell the app to quit once we're out of the
  // mixer lock, otherwise the windows driver will never return

  _shouldQuit = false;

  _isDirty = false;

  unsigned short v = 1 << event.GetValue();

  MixerService *sm = MixerService::GetInstance();
  sm->Lock();

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
       (event.GetValue()==EKT_ESCAPE&&!Player::GetInstance()->IsRunning()) { if
       (_currentView!=_listView) { CloseProject() ; _isDirty=true ; } else {
                            System::GetInstance()->PostQuitMessage() ;
                    };
            } ;*/

  default:
    break;
  }
  sm->Unlock();

  if (_shouldQuit) {
    onQuitApp();
  }
  if (_closeProject) {
    CloseProject();
    _isDirty = true;
  }
#ifdef _SHOW_GP2X_
  Redraw();
#else
  if (_isDirty)
    Redraw();
#endif
  return false;
};

void AppWindow::onUpdate() {
  //	Redraw() ;
  Flush();
};

void AppWindow::AnimationUpdate() { _currentView->AnimationUpdate(); }

void AppWindow::LayoutChildren(){};

void AppWindow::Update(Observable &o, I_ObservableData *d) {

  ViewEvent *ve = (ViewEvent *)d;

  switch (ve->GetType()) {

  case VET_SWITCH_VIEW: {
    ViewType *vt = (ViewType *)ve->GetData();
    printf("AppWindow CHANGE VIEW:%d %d\n", &vt, _currentView);
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
    default:
      break;
    }
    _currentView->SetFocus(*vt);
    _isDirty = true;
    GUIWindow::Clear(backgroundColor_, true);
    Clear(true);
    Redraw();
    break;
  }

  case VET_PLAYER_POSITION_UPDATE: {
    PlayerEvent *pt = (PlayerEvent *)ve;
    if (_currentView) {
      SysMutexLocker locker(drawMutex_);
      _currentView->OnPlayerUpdate(pt->GetType(), pt->GetTickCount());
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
void AppWindow::Print(char *line) {

  //	GUIWindow::Clear(View::backgroundColor_,true) ;
  Clear();
  strcpy(_statusLine, line);
  // unwrapped for gcc
  int position = 32;
  position -= strlen(_statusLine);
  position /= 2;
  GUIPoint pos(position, 12);
  //
  GUITextProperties props;
  SetColor(CD_NORMAL);
  DrawString(_statusLine, pos, props);
  char buildString[80];
  sprintf(buildString, "picoTracker build %s%s_%s", PROJECT_NUMBER,
          PROJECT_RELEASE, BUILD_COUNT);
  pos._y = 22;
  pos._x = (32 - strlen(buildString)) / 2;
  DrawString(buildString, pos, props);
  Flush();
};

void AppWindow::SetColor(ColorDefinition cd) { colorIndex_ = cd; };
