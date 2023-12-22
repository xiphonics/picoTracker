#include "ProjectView.h"
#include "Application/Model/Scale.h"
#include "Application/Persistency/PersistencyService.h"
#include "Application/Views/ModalDialogs/MessageBox.h"
#include "Application/Views/ModalDialogs/NewProjectDialog.h"
#include "Application/Views/ModalDialogs/SelectProjectDialog.h"
#include "BaseClasses/UIActionField.h"
#include "BaseClasses/UIIntVarField.h"
#include "BaseClasses/UITempoField.h"
#include "Services/Midi/MidiService.h"
#include "System/System/System.h"
#ifdef PICOBUILD
#include "hardware/watchdog.h"
#include "pico/bootrom.h"
#endif

#define ACTION_PURGE MAKE_FOURCC('P', 'U', 'R', 'G')
#define ACTION_SAVE MAKE_FOURCC('S', 'A', 'V', 'E')
#define ACTION_SAVE_AS MAKE_FOURCC('S', 'V', 'A', 'S')
#define ACTION_LOAD MAKE_FOURCC('L', 'O', 'A', 'D')
#define ACTION_BOOTSEL MAKE_FOURCC('B', 'O', 'O', 'T')
#define ACTION_PURGE_INSTRUMENT MAKE_FOURCC('P', 'R', 'G', 'I')
#define ACTION_TEMPO_CHANGED MAKE_FOURCC('T', 'E', 'M', 'P')

void ProjectView::Toast(const char *text) {
  MessageBox *mb = new MessageBox(*this, text, MBBF_OK);
  DoModal(mb);
}

static void SaveAsProjectCallback(View &v, ModalView &dialog) {
  FileSystemService FSS;
  NewProjectDialog &npd = (NewProjectDialog &)dialog;
  Trace::Log("ProjectView", "Dialog result: %d", dialog.GetReturnCode());

  if (dialog.GetReturnCode() > 0) {
    std::string strDestProject;
    std::string strDestSamples;

    Path root("root:");
    strDestProject = root.GetName() + npd.GetName();
    strDestSamples = strDestProject + "/samples/";

    Path pathDestProject = Path(strDestProject);
    Path pathDestSamples = Path(strDestSamples);
    Path pathSourceProject("project:");
    Path pathSourceSamples("project:samples");

    Path pathSourceSave = pathSourceProject.GetPath() + "lgptsav.dat";
    Path pathDestSav = pathDestProject.GetPath() + "/lgptsav.dat";
    if (pathDestProject.Exists()) {
      const char *errMsg =
          ("Project '" + pathDestProject.GetPath() + "' already exists")
              .c_str();
      Trace::Log("ProjectView", errMsg);
    } else {
      Result result =
          FileSystem::GetInstance()->MakeDir(pathDestProject.GetPath().c_str());
      if (result.Failed()) {
        const char *errMsg =
            ("Failed to create dir '" + pathDestProject.GetPath() + "'")
                .c_str();
        Trace::Log("ProjectView", errMsg);
        return;
      };
      result =
          FileSystem::GetInstance()->MakeDir(pathDestSamples.GetPath().c_str());
      if (result.Failed()) {
        const char *errMsg =
            ("Failed to create sample dir '" + pathDestProject.GetPath() + "'")
                .c_str();
        Trace::Log("ProjectView", errMsg);
        return;
      };

      FSS.Copy(pathSourceSave, pathDestSav);

      I_Dir *fileList =
          FileSystem::GetInstance()->Open(pathSourceSamples.GetPath().c_str());
      if (fileList) {
        fileList->GetContent("*");
        for (fileList->Begin(); !fileList->IsDone(); fileList->Next()) {
          Path &current = fileList->CurrentItem();
          if (current.IsFile()) {
            Path dstfile = Path((strDestSamples + current.GetName()).c_str());
            Path srcfile = Path(current.GetPath());
            FSS.Copy(srcfile.GetPath(), dstfile.GetPath());
            Trace::Log("ProjectView", "OK copy %s", srcfile.GetPath().c_str());
          }
        }
      }
      if (dialog.GetReturnCode() == 1) {
#ifdef PICOBUILD
        // TODO: Remove this hack. Due to memory leaks and other problems
        // instead of going back, we perform a software reset
        watchdog_reboot(0, 0, 0);
#endif
        ((ProjectView &)v).OnSaveAsProject((char *)strDestProject.c_str());
      }
    }
  }
}

static void LoadCallback(View &v, ModalView &dialog) {
  if (dialog.GetReturnCode() == MBL_YES) {
#ifdef PICOBUILD
    // TODO: Remove this hack. Due to memory leaks and other problems
    // instead of going back, we perform a software reset
    watchdog_reboot(0, 0, 0);
#endif
    ((ProjectView &)v).OnLoadProject();
  }
};

#ifdef PICOBUILD
static void BootselCallback(View &v, ModalView &dialog) {
  if (dialog.GetReturnCode() == MBL_YES) {
    reset_usb_boot(0, 0);
  }
};
#endif

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
  a1 = new UIActionField("Load Song", ACTION_LOAD, position);
  a1->AddObserver(*this);
  fieldList_.insert(fieldList_.end(), a1);

  position._y += 1;
  a1 = new UIActionField("Save Song", ACTION_SAVE, position);
  a1->AddObserver(*this);
  fieldList_.insert(fieldList_.end(), a1);

  position._y += 1;
  a1 = new UIActionField("Save Song As", ACTION_SAVE_AS, position);
  a1->AddObserver(*this);
  fieldList_.insert(fieldList_.end(), a1);

  v = project_->FindVariable(VAR_MIDIDEVICE);
  NAssert(v);
  position._y += 2;
  UIIntVarField *f4 = new UIIntVarField(
      position, *v, "MIDI: %s", 0, MidiService::GetInstance()->Size(), 1, 1);
  fieldList_.insert(fieldList_.end(), f4);

  position._y += 2;
  a1 = new UIActionField("Update firmware", ACTION_BOOTSEL, position);
  a1->AddObserver(*this);
  fieldList_.insert(fieldList_.end(), a1);
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
  case ACTION_SAVE: {
    if (!player->IsRunning()) {
      PersistencyService *service = PersistencyService::GetInstance();
      service->Save();
    } else {
      MessageBox *mb = new MessageBox(*this, "Not while playing", MBBF_OK);
      DoModal(mb);
    }
    break;
  }
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
  case ACTION_SAVE_AS: {
    Toast("enter case");
    if (!player->IsRunning()) {
      PersistencyService *service = PersistencyService::GetInstance();
      service->Save();
      NewProjectDialog *mb = new NewProjectDialog(*this);
      DoModal(mb, SaveAsProjectCallback);
    } else {
      MessageBox *mb = new MessageBox(*this, "Not while playing", MBBF_OK);
      DoModal(mb);
    }
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

void ProjectView::OnLoadProject() {
  ViewEvent ve(VET_QUIT_PROJECT);
  SetChanged();
  NotifyObservers(&ve);
};

void ProjectView::OnSaveAsProject(char *data) {
  ViewEvent ve(VET_SAVEAS_PROJECT, data);
  SetChanged();
  NotifyObservers(&ve);
};

void ProjectView::OnQuit() {
  ViewEvent ve(VET_QUIT_APP);
  SetChanged();
  NotifyObservers(&ve);
};
