#include "ProjectView.h"
#include "Application/Model/Scale.h"
#include "Application/Persistency/PersistencyService.h"
#include "Application/Utils/randomnames.h"
#include "Application/Views/ModalDialogs/MessageBox.h"
#include "BaseClasses/UIActionField.h"
#include "BaseClasses/UIIntVarField.h"
#include "BaseClasses/UITempoField.h"
#include "BaseClasses/View.h"
#include "BaseClasses/ViewEvent.h"
#include "Services/Midi/MidiService.h"
#include "System/System/System.h"
#ifdef PICOBUILD
#include "hardware/watchdog.h"
#include "pico/bootrom.h"
#endif

#define ACTION_PURGE MAKE_FOURCC('P', 'U', 'R', 'G')
#define ACTION_SAVE MAKE_FOURCC('S', 'A', 'V', 'E')
#define ACTION_LOAD MAKE_FOURCC('L', 'O', 'A', 'D')
#define ACTION_BOOTSEL MAKE_FOURCC('B', 'O', 'O', 'T')
#define ACTION_PURGE_INSTRUMENT MAKE_FOURCC('P', 'R', 'G', 'I')
#define ACTION_TEMPO_CHANGED MAKE_FOURCC('T', 'E', 'M', 'P')
#define ACTION_RANDOM_NAME MAKE_FOURCC('R', 'N', 'D', 'P')
#define ACTION_NEW_PROJECT MAKE_FOURCC('N', 'E', 'W', 'P')

#define ACTION_PROJECT_RENAME MAKE_FOURCC('P', 'R', 'N', 'M')
static void LoadCallback(View &v, ModalView &dialog) {
  if (dialog.GetReturnCode() == MBL_YES) {
    ViewType vt = VT_SELECTPROJECT;
    ViewEvent ve(VET_SWITCH_VIEW, &vt);
    ((ProjectView &)v).SetChanged();
    ((ProjectView &)v).NotifyObservers(&ve);
  }
};

static void CreateNewProjectCallback(View &v, ModalView &dialog) {
  if (dialog.GetReturnCode() == MBL_YES) {
    PersistencyService::GetInstance()->SaveProjectState(UNNAMED_PROJECT_NAME);

    // now reboot!
    watchdog_reboot(0, 0, 0);
  }
};

#ifdef PICOBUILD
static void BootselCallback(View &v, ModalView &dialog) {
  if (dialog.GetReturnCode() == MBL_YES) {
    reset_usb_boot(0, 0);
  }
};
#endif

static void SaveAsOverwriteCallback(View &v, ModalView &dialog) {
  bool cancelOverwrite = dialog.GetReturnCode() == MBL_CANCEL;
  if (cancelOverwrite) {
    return;
  }

  PersistencyService *persist = PersistencyService::GetInstance();
  const char *projName = ((ProjectView &)v).getProjectName().c_str();
  if (persist->Save(projName, true) != PERSIST_SAVED) {
    Trace::Error("failed to save project");
    MessageBox *mb = new MessageBox(
        ((ProjectView &)v), "failed to save project", MBBF_OK | MBBF_CANCEL);
    ((ProjectView &)v).DoModal(mb, SaveAsOverwriteCallback);
    return;
  }
  if (persist->SaveProjectState(projName) != PERSIST_SAVED) {
    Trace::Error("Failed to save project state");
  } else {
    Trace::Log("PROJECTVIEW-STATIC", "OVERWROTE:%s", projName);
    ((ProjectView &)v).clearSaveAsFlag(); // clear flag after saving
  }
}

static void PurgeCallback(View &v, ModalView &dialog) {
  ((ProjectView &)v).OnPurgeInstruments(dialog.GetReturnCode() == MBL_YES);
};

ProjectView::ProjectView(GUIWindow &w, ViewData *data) : FieldView(w, data) {

  lastClock_ = 0;
  lastTick_ = 0;

  project_ = data->project_;

  GUIPoint position = GetAnchor();

  Variable *v = project_->FindVariable(VAR_TEMPO);
  UITempoField *f = new UITempoField(ACTION_TEMPO_CHANGED, position, *v,
                                     "tempo: %d [%2.2x]  ", 60, 400, 1, 10);
  fieldList_.insert(fieldList_.end(), f);
  f->AddObserver(*this);
  tempoField_ = f;

  v = project_->FindVariable(VAR_MASTERVOL);
  position._y += 1;
  UIIntVarField *f1 =
      new UIIntVarField(position, *v, "master: %d", 10, 200, 1, 10);
  fieldList_.insert(fieldList_.end(), f1);

  v = project_->FindVariable(VAR_TRANSPOSE);
  position._y += 1;
  UIIntVarField *f2 =
      new UIIntVarField(position, *v, "transpose: %3.2d", -48, 48, 0x1, 0xC);
  fieldList_.insert(fieldList_.end(), f2);

  v = project_->FindVariable(VAR_SCALE);
  // if scale name is not found, set the default chromatic scale
  if (v->GetInt() < 0) {
    v->SetInt(0);
  }
  position._y += 1;
  UIIntVarField *f3 =
      new UIIntVarField(position, *v, "scale: %s", 0, numScales - 1, 1, 10);
  fieldList_.insert(fieldList_.end(), f3);

  position._y += 2;
  UIActionField *a1 =
      new UIActionField("Compact Sequencer", ACTION_PURGE, position);
  a1->AddObserver(*this);
  fieldList_.insert(fieldList_.end(), a1);

  position._y += 1;
  a1 = new UIActionField("Compact Instruments", ACTION_PURGE_INSTRUMENT,
                         position);
  a1->AddObserver(*this);
  fieldList_.insert(fieldList_.end(), a1);

  position._y += 2;

  // save existing fields horizontal alignment
  int xalign = position._x;

  v = project_->FindVariable(VAR_PROJECTNAME);
  nameField_ = new UITextField(v, position, "project: ", ACTION_PROJECT_RENAME);
  nameField_->AddObserver(*this);
  fieldList_.insert(fieldList_.end(), nameField_);

  position._y += 1;
  a1 = new UIActionField("Load", ACTION_LOAD, position);
  a1->AddObserver(*this);
  fieldList_.insert(fieldList_.end(), a1);

  position._x += 5;
  a1 = new UIActionField("Save", ACTION_SAVE, position);
  a1->AddObserver(*this);
  fieldList_.insert(fieldList_.end(), a1);

  position._x += 5;
  a1 = new UIActionField("New", ACTION_NEW_PROJECT, position);
  a1->AddObserver(*this);
  fieldList_.insert(fieldList_.end(), a1);

  position._x += 5;
  a1 = new UIActionField("Random", ACTION_RANDOM_NAME, position);
  a1->AddObserver(*this);
  fieldList_.insert(fieldList_.end(), a1);
  position._x = xalign;

  v = project_->FindVariable(VAR_MIDIDEVICE);
  NAssert(v);
  position._y += 2;
  UIIntVarField *f4 = new UIIntVarField(
      position, *v, "midi: %s", 0, MidiService::GetInstance()->Size(), 1, 1);
  fieldList_.insert(fieldList_.end(), f4);
}

ProjectView::~ProjectView() {}

void ProjectView::ProcessButtonMask(unsigned short mask, bool pressed) {

  if (!pressed)
    return;

  FieldView::ProcessButtonMask(mask);

  if (mask & EPBM_R) {
    if (mask & EPBM_DOWN) {
      ViewType vt = VT_SONG;
      ViewEvent ve(VET_SWITCH_VIEW, &vt);
      SetChanged();
      NotifyObservers(&ve);
    }
    if (mask & EPBM_UP) {
      ViewType vt = VT_MACHINE;
      ViewEvent ve(VET_SWITCH_VIEW, &vt);
      SetChanged();
      NotifyObservers(&ve);
    }
  } else {
    if (mask & EPBM_START) {
      Player *player = Player::GetInstance();
      player->OnStartButton(PM_SONG, viewData_->songX_, false,
                            viewData_->songX_);
    }
  };
};

void ProjectView::DrawView() {

  Clear();

  GUITextProperties props;
  GUIPoint pos = GetTitlePosition();

  // Draw title
  char projectString[80];
  sprintf(projectString, "Project - Build %s%s_%s", PROJECT_NUMBER,
          PROJECT_RELEASE, BUILD_COUNT);

  SetColor(CD_NORMAL);
  DrawString(pos._x, pos._y, projectString, props);

  FieldView::Redraw();
  drawMap();
};

void ProjectView::Update(Observable &, I_ObservableData *data) {

  if (!hasFocus_) {
    return;
  }

  uintptr_t fourcc = (uintptr_t)data;

  UIField *focus = GetFocus();
  if (fourcc != ACTION_TEMPO_CHANGED) {
    focus->ClearFocus();
    focus->Draw(w_);
    w_.Flush();
    focus->SetFocus();
  } else {
    focus = tempoField_;
  }
  Player *player = Player::GetInstance();

  switch (fourcc) {
  case ACTION_PURGE:
    project_->Purge();
    break;
  case ACTION_PURGE_INSTRUMENT: {
    MessageBox *mb = new MessageBox(*this, "Purge unused samples from disk ?",
                                    MBBF_YES | MBBF_NO);
    DoModal(mb, PurgeCallback);
    break;
  }
  case ACTION_RANDOM_NAME:
    char name[12];
    getRandomName(name);
    printf("random:%s", name);
    project_->SetProjectName(name);
    saveAsFlag_ = true;
    break;
  case ACTION_SAVE:
    if (!player->IsRunning()) {
      PersistencyService *persist = PersistencyService::GetInstance();
      char projName[MAX_PROJECT_NAME_LENGTH];
      project_->GetProjectName(projName);

      if (strcmp(projName, UNNAMED_PROJECT_NAME) == 0) {
        MessageBox *mb =
            new MessageBox(*this, "Please name the project first", MBBF_OK);
        DoModal(mb);
        return;
      }
      if (saveAsFlag_) {
        // first need to check if project with this name already exists
        if (persist->Exists(projName)) {
          Trace::Error("project already exists ask user to confirm overwrite");
          MessageBox *mb = new MessageBox(*this, "Overwrite EXISTING project?",
                                          MBBF_OK | MBBF_CANCEL);
          DoModal(mb, SaveAsOverwriteCallback);
          return;
        }
        if (persist->Save(projName, saveAsFlag_) != PERSIST_SAVED) {
          Trace::Error("failed to save project state");
          MessageBox *mb =
              new MessageBox(*this, "Error saving Project", MBBF_OK);
          DoModal(mb);
          return;
        }
        saveAsFlag_ = false; // clear flag after saving
      } else {
        // all good so now persist the new project name in project state
        persist->SaveProjectState(projName);
      }
    } else {
      MessageBox *mb = new MessageBox(*this, "Not while playing", MBBF_OK);
      DoModal(mb);
    }
    break;
  case ACTION_PROJECT_RENAME:
    Trace::Log("PROJECTVIEW", "Project renamed! prev name:%s",
               nameField_->GetString().c_str());
    saveAsFlag_ = true;
    break;
  case ACTION_LOAD: {
    if (!player->IsRunning()) {
      MessageBox *mb = new MessageBox(*this, "Load song and lose changes?",
                                      MBBF_YES | MBBF_NO);
      DoModal(mb, LoadCallback);
    } else {
      MessageBox *mb = new MessageBox(*this, "Not while playing", MBBF_OK);
      DoModal(mb);
    }
    break;
  }
  case ACTION_NEW_PROJECT: {
    MessageBox *mb = new MessageBox(*this, "New project and lose changes?",
                                    MBBF_YES | MBBF_NO);
    DoModal(mb, CreateNewProjectCallback);
    break;
  }
#ifdef PICOBUILD
  case ACTION_BOOTSEL: {
    if (!player->IsRunning()) {
      MessageBox *mb =
          new MessageBox(*this, "Reboot and lose changes?", MBBF_YES | MBBF_NO);
      DoModal(mb, BootselCallback);
    } else {
      MessageBox *mb = new MessageBox(*this, "Not while playing", MBBF_OK);
      DoModal(mb);
    }
    break;
  }
#endif

  case ACTION_TEMPO_CHANGED:
    break;
  default:
    NInvalid;
    break;
  };
  focus->Draw(w_);
  isDirty_ = true;
};

void ProjectView::OnPurgeInstruments(bool removeFromDisk) {
  project_->PurgeInstruments(removeFromDisk);
};

void ProjectView::OnQuit() {
  ViewEvent ve(VET_QUIT_APP);
  SetChanged();
  NotifyObservers(&ve);
};
