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
#include <nanoprintf.h>

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

    // first clear out any existing "unnamed" project
    PersistencyService::GetInstance()->PurgeUnnamedProject();

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
  const char *oldProjName = ((ProjectView &)v).getOldProjectName().c_str();

  if (persist->Save(projName, oldProjName, true) != PERSIST_SAVED) {
    Trace::Error("failed to save renamed project %s [old: %s]", projName,
                 oldProjName);
    MessageBox *mb = new MessageBox(
        ((ProjectView &)v), "Failed to save project", MBBF_OK | MBBF_CANCEL);
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

  Variable *v = project_->FindVariable(FourCC::VarTempo);
  UITempoField *f =
      new UITempoField(FourCC::ActionTempoChanged, position, *v,
                       "tempo: %d [%2.2x]  ", MIN_TEMPO, MAX_TEMPO, 1, 10);
  fieldList_.insert(fieldList_.end(), f);
  f->AddObserver(*this);
  tempoField_ = f;

  v = project_->FindVariable(FourCC::VarMasterVolume);
  position._y += 1;
  UIIntVarField *f1 =
      new UIIntVarField(position, *v, "master: %d", 10, 200, 1, 10);
  fieldList_.insert(fieldList_.end(), f1);

  v = project_->FindVariable(FourCC::VarTranspose);
  position._y += 1;
  UIIntVarField *f2 =
      new UIIntVarField(position, *v, "transpose: %3.2d", -48, 48, 0x1, 0xC);
  fieldList_.insert(fieldList_.end(), f2);

  v = project_->FindVariable(FourCC::VarScale);
  // if scale name is not found, set the default chromatic scale
  if (v->GetInt() < 0) {
    v->SetInt(0);
  }
  position._y += 1;
  UIIntVarField *f3 =
      new UIIntVarField(position, *v, "scale: %s", 0, numScales - 1, 1, 10);
  fieldList_.insert(fieldList_.end(), f3);

  position._y += 2;
  UIActionField *a1 = new UIActionField(
      "Compact Instruments", FourCC::ActionPurgeInstrument, position);
  a1->AddObserver(*this);
  fieldList_.insert(fieldList_.end(), a1);

  position._y += 2;

  // save existing fields horizontal alignment
  int xalign = position._x;

  v = project_->FindVariable(FourCC::VarProjectName);
  auto label = etl::make_string_with_capacity<MAX_LABEL_LENGTH>("project: ");
  auto defaultName = etl::make_string_with_capacity<MAX_PROJECT_NAME_LENGTH>(
      UNNAMED_PROJECT_NAME);
  nameField_ = new UITextField<MAX_PROJECT_NAME_LENGTH>(
      *v, position, label, FourCC::ActionProjectRename, defaultName);

  nameField_->AddObserver(*this);
  fieldList_.insert(fieldList_.end(), nameField_);

  position._y += 1;
  a1 = new UIActionField("Load", FourCC::ActionLoad, position);
  a1->AddObserver(*this);
  fieldList_.insert(fieldList_.end(), a1);

  position._x += 5;
  a1 = new UIActionField("Save", FourCC::ActionSave, position);
  a1->AddObserver(*this);
  fieldList_.insert(fieldList_.end(), a1);

  position._x += 5;
  a1 = new UIActionField("New", FourCC::ActionNewProject, position);
  a1->AddObserver(*this);
  fieldList_.insert(fieldList_.end(), a1);

  position._x += 5;
  a1 = new UIActionField("Random", FourCC::ActionRandomName, position);
  a1->AddObserver(*this);
  fieldList_.insert(fieldList_.end(), a1);
  position._x = xalign;
}

ProjectView::~ProjectView() {}

void ProjectView::ProcessButtonMask(unsigned short mask, bool pressed) {

  if (!pressed)
    return;

  FieldView::ProcessButtonMask(mask);

  if (mask & EPBM_NAV) {
    if (mask & EPBM_DOWN) {
      ViewType vt = VT_SONG;
      ViewEvent ve(VET_SWITCH_VIEW, &vt);
      SetChanged();
      NotifyObservers(&ve);
    }
    if (mask & EPBM_UP) {
      ViewType vt = VT_DEVICE;
      ViewEvent ve(VET_SWITCH_VIEW, &vt);
      SetChanged();
      NotifyObservers(&ve);
    }
  } else {
    if (mask & EPBM_PLAY) {
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

  SetColor(CD_NORMAL);

  // Draw title
  const char *title = "Project ";
  DrawString(pos._x, pos._y, title, props);

  Variable *v = viewData_->project_->FindVariable(FourCC::VarProjectName);
  etl::string<MAX_PROJECT_NAME_LENGTH> projectName = v->GetString();
  DrawString(pos._x + strlen(title), pos._y, projectName.c_str(), props);

  drawBattery(props);

  FieldView::Redraw();
  drawMap();
};

void ProjectView::Update(Observable &, I_ObservableData *data) {

  if (!hasFocus_) {
    return;
  }

  uintptr_t fourcc = (uintptr_t)data;

  UIField *focus = GetFocus();
  if (fourcc != FourCC::ActionTempoChanged) {
    focus->ClearFocus();
    focus->Draw(w_);
    w_.Flush();
    focus->SetFocus();
  } else {
    focus = tempoField_;
  }
  Player *player = Player::GetInstance();

  switch (fourcc) {
  case FourCC::ActionPurge:
    project_->Purge();
    break;
  case FourCC::ActionPurgeInstrument: {
    MessageBox *mb =
        new MessageBox(*this, "Remove unused samples?", MBBF_YES | MBBF_NO);
    DoModal(mb, PurgeCallback);
    break;
  }
  case FourCC::ActionRandomName:
    char name[12];
    getRandomName(name);
    printf("random:%s", name);
    project_->SetProjectName(name);
    saveAsFlag_ = true;
    break;
  case FourCC::ActionSave:
    if (!player->IsRunning()) {
      PersistencyService *persist = PersistencyService::GetInstance();
      char projName[MAX_PROJECT_NAME_LENGTH];
      project_->GetProjectName(projName);

      if (saveAsFlag_) {
        // first need to check if project with this name already exists
        if (persist->Exists(projName)) {
          Trace::Error("project already exists ask user to confirm overwrite");
          MessageBox *mb = new MessageBox(*this, "Overwrite EXISTING project?",
                                          MBBF_OK | MBBF_CANCEL);
          DoModal(mb, SaveAsOverwriteCallback);
          return;
        }
        if (persist->Save(projName, oldProjName_.c_str(), saveAsFlag_) !=
            PERSIST_SAVED) {
          Trace::Error("failed to save project state");
          MessageBox *mb =
              new MessageBox(*this, "Error saving Project", MBBF_OK);
          DoModal(mb);
          return;
        }
        clearSaveAsFlag();
      } else {
        if (persist->Save(projName, oldProjName_.c_str(), saveAsFlag_) !=
            PERSIST_SAVED) {
          Trace::Error("failed to save project state");
          MessageBox *mb =
              new MessageBox(*this, "Error saving Project", MBBF_OK);
          DoModal(mb);
          return;
        }
      }
      // all good so now persist the new project name in project state
      persist->SaveProjectState(projName);
    } else {
      MessageBox *mb = new MessageBox(*this, "Not while playing", MBBF_OK);
      DoModal(mb);
    }
    break;
  case FourCC::ActionProjectRename:
    Trace::Log("PROJECTVIEW", "Project renamed! prev name:%s",
               nameField_->GetString().c_str());
    saveAsFlag_ = true;
    break;
  case FourCC::ActionLoad: {
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
  case FourCC::ActionNewProject: {
    MessageBox *mb = new MessageBox(*this, "New project and lose changes?",
                                    MBBF_YES | MBBF_NO);
    DoModal(mb, CreateNewProjectCallback);
    break;
  }
#ifdef PICOBUILD
  case FourCC::ActionBootSelect: {
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

  case FourCC::ActionTempoChanged:
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

void ProjectView::OnFocus() {
  // only store current project name for use in a "save as" operation if it's
  // not already been modified by the user pending a save which is indicated
  // by the saveAsFlag_ flag being set
  if (!saveAsFlag_) {
    oldProjName_ = getProjectName();
  }
}

/// Updates the animation by redrawing the battery gauge on every clock tick
/// (~1Hz). This occurs even when playback is not active and there is no user
/// cursor navigation.
void ProjectView::AnimationUpdate() {
  // redraw batt gauge on every clock tick (~1Hz) even when not playing
  // and not redrawing due to user cursor navigation
  GUITextProperties props;
  drawBattery(props);
  w_.Flush();
};