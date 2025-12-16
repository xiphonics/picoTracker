/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "SelectProjectView.h"
#include "Application/AppWindow.h"
#include "Application/Persistency/PersistencyService.h"
#include "Application/Views/ModalDialogs/MessageBox.h"
#include <nanoprintf.h>

#define LIST_PAGE_SIZE (SCREEN_HEIGHT - 4)
#define LIST_WIDTH 26
#define INVALID_PROJECT_NAME "INVALID NAME"

static void ConfirmOverwriteCallback(View &v, ModalView &dialog) {
  if (dialog.GetReturnCode() == MBL_YES) {
    ((SelectProjectView &)v).SaveSelectedProject();
  }
}

static void DeleteProjectCallback(View &v, ModalView &dialog) {
  if (dialog.GetReturnCode() == MBL_YES) {
    // delete project
    PersistencyService *ps = PersistencyService::GetInstance();
    char buffer[MAX_PROJECT_NAME_LENGTH + 1];
    ((SelectProjectView &)v).getHighlightedProjectName(buffer);
    ps->DeleteProject(buffer);

    // reload list
    ((SelectProjectView &)v).setCurrentFolder();
  }
}

SelectProjectView::SelectProjectView(GUIWindow &w, ViewData *viewData)
    : ScreenView(w, viewData) {}

SelectProjectView::~SelectProjectView() {}

void SelectProjectView::DrawView() {
  Clear();

  GUITextProperties props;
  GUIPoint pos = GetTitlePosition();
  SetColor(CD_NORMAL);

  auto fs = FileSystem::GetInstance();

  // Draw title
  const char *title = "Browse Projects";
  SetColor(CD_INFO);
  DrawString(pos._x + 1, pos._y, title, props);
  SetColor(CD_NORMAL);

  // Draw projects
  int x = 1;
  int y = pos._y + 2;

  auto var = viewData_->project_->FindVariable(FourCC::VarProjectName);
  etl::string<MAX_PROJECT_NAME_LENGTH> projectName = var->GetString();
  const char *currentProject = projectName.c_str();

  for (size_t i = topIndex_;
       i < topIndex_ + LIST_PAGE_SIZE && (i < fileIndexList_.size()); i++) {
    if (i == currentIndex_) {
      SetColor(CD_HILITE2);
      props.invert_ = true;
    } else {
      SetColor(CD_NORMAL);
      props.invert_ = false;
    }

    char buffer[MAX_PROJECT_NAME_LENGTH + 1];
    memset(buffer, '\0', sizeof(buffer));
    unsigned fileIndex = fileIndexList_[i];

    if (fs->getFileType(fileIndex) == PFT_DIR) {
      fs->getFileName(fileIndex, buffer, MAX_PROJECT_NAME_LENGTH + 1);
    }
    // SDFat lib doesn't truncate if filename longer than buffer as per docs but
    // instead returns empty string in buffer
    if (strlen(buffer) == 0) {
      strcpy(buffer, INVALID_PROJECT_NAME);
    }

    if (strcmp(buffer, currentProject) == 0) {
      // mark currently loaded project
      DrawString(x - 1, y, "*", props);
    }

    DrawString(x, y, buffer, props);
    y += 1;
  };

  // load/delete selection buttons
  const char *buttons[numButtons_] = {
      "Load", SelectionIsCurrentProject() ? "Save" : "Save as", "Delete"};

  for (int n = 0; n < numButtons_; n++) {
    bool selected = selectedButton_ == n;
    props.invert_ = selected;
    SetColor(selected ? CD_HILITE2 : CD_HILITE1);
    DrawString(x + 10 * n, SCREEN_HEIGHT - 1, buttons[n], props);
  }

  // scroll bar
  DrawScrollBar();
};

void SelectProjectView::DrawScrollBar() {
  int totalItems = fileIndexList_.size();
  if (totalItems <= LIST_PAGE_SIZE) {
    return; // no scrollbar needed
  }

  GUITextProperties props;
  GUITextProperties inv;
  inv.invert_ = true;
  SetColor(CD_NORMAL);

  // Thumb size represents the ratio of visible items to total items
  int thumbSize = std::max(1, (LIST_PAGE_SIZE * LIST_PAGE_SIZE) / totalItems);

  // Thumb position: map topIndex (0 to maxScroll) onto available scrollbar
  // space
  int maxScroll = totalItems - LIST_PAGE_SIZE;
  int availableSpace = LIST_PAGE_SIZE - thumbSize;
  int thumbPos = (topIndex_ * availableSpace) / maxScroll;

  Trace::Error("%d total, %d thumb size, %d maxScroll, %d thumbPos", totalItems,
               thumbSize, maxScroll, thumbPos);
  for (int y = 0; y < LIST_PAGE_SIZE; y++) {
    bool thumb = y >= thumbPos && y < thumbPos + thumbSize;
    DrawString(SCREEN_WIDTH - 1, 2 + y, thumb ? " " : "|", thumb ? inv : props);
  }
}

void SelectProjectView::OnPlayerUpdate(PlayerEventType,
                                       unsigned int currentTick){};

void SelectProjectView::OnFocus() { setCurrentFolder(); };

void SelectProjectView::DeleteProject() {
  if (currentIndex_ >= fileIndexList_.size()) {
    return;
  }

  char selected[MAX_PROJECT_NAME_LENGTH + 1];
  getHighlightedProjectName(selected);

  // make sure we are not attempting to delete the current project
  auto var = viewData_->project_->FindVariable(FourCC::VarProjectName);
  etl::string<MAX_PROJECT_NAME_LENGTH> currentProject = var->GetString();
  const char *current = currentProject.c_str();

  if (strcmp(current, selected) == 0) {
    MessageBox *mb =
        new MessageBox(*this, "Cannot delete the current project.", MBBF_OK);
    DoModal(mb);
    return;
  }

  char buffer[MAX_PROJECT_NAME_LENGTH + 11];
  npf_snprintf(buffer, sizeof(buffer), "Delete \"%s\"?", selected);

  MessageBox *mb = new MessageBox(*this, buffer, MBBF_YES | MBBF_NO);
  DoModal(mb, DeleteProjectCallback);
}

void SelectProjectView::ProcessButtonMask(unsigned short mask, bool pressed) {
  if (!pressed)
    return;

  if (mask & EPBM_EDIT) {
    // EDIT+ENTER -> hotkey to delete
    if (mask & EPBM_ENTER)
      AttemptDeletingSelectedProject();
    if (mask & EPBM_UP)
      warpToNextProject(true);
    if (mask & EPBM_DOWN)
      warpToNextProject(false);
  } else {
    // A modifier
    if (mask & EPBM_ENTER) {
      switch (selectedButton_) {
      case 0:
        // load project
        if (!WarnPlayerRunning()) {
          LoadProject();
        }
        break;
      case 1:
        // save project
        if (!SelectionIsCurrentProject()) {
          // ask if the user wants to override the file
          ConfirmOverwrite();
        } else {
          // save
          SaveSelectedProject();
        }
        break;
      case 2:
        AttemptDeletingSelectedProject();
        break;
      }
    } else {
      // R Modifier
      if ((mask & EPBM_NAV) && (mask & EPBM_LEFT)) {
        // Go to back "left" to Project Screen
        ViewType vt = VT_PROJECT;
        ViewEvent ve(VET_SWITCH_VIEW, &vt);
        SetChanged();
        NotifyObservers(&ve);
        return;
      } else {
        // No modifier
        if (mask == EPBM_UP)
          warpToNextProject(true);
        if (mask == EPBM_DOWN)
          warpToNextProject(false);
        if (mask == EPBM_LEFT)
          SelectButton(-1);
        if (mask == EPBM_RIGHT)
          SelectButton(1);
      }
    }
  }
}

void SelectProjectView::warpToNextProject(bool goUp) {

  if (goUp) {
    if (currentIndex_ > 0) {
      currentIndex_--;
      // if we have scrolled off the top, page the file list up if not
      // already at  very top of the list
      if (currentIndex_ < topIndex_) {
        topIndex_ = currentIndex_;
      }
    }
  } else {
    if (currentIndex_ < fileIndexList_.size() - 1) {
      currentIndex_++;
      // if we have scrolled off the bottom, page the file list down if not
      // at end of the list
      if (currentIndex_ >= (topIndex_ + LIST_PAGE_SIZE)) {
        topIndex_++;
      }
    }
  }
  isDirty_ = true;
}

void SelectProjectView::setCurrentFolder() {
  auto fs = FileSystem::GetInstance();
  fs->chdir(PROJECTS_DIR);

  // get ready
  fileIndexList_.clear();

  // Let's read all the directory in the project dir
  fs->list(&fileIndexList_, "", true);

  // temp hack,  filter out "." & ".."
  fileIndexList_.erase(fileIndexList_.begin());
  fileIndexList_.erase(fileIndexList_.begin());

  // filter out the "untitled" project entry
  for (size_t i = 0; i < fileIndexList_.size(); i++) {
    fs->getFileName(fileIndexList_[i], selection_, MAX_PROJECT_NAME_LENGTH + 1);
    if (strcmp(selection_, UNNAMED_PROJECT_NAME) == 0) {
      Trace::Log("SELECTPROJECTVIEW", "skipping untitled project on Index:%d",
                 i);
      fileIndexList_.erase(fileIndexList_.begin() + i);
      break;
    }
  }

  // reset & redraw screen
  currentIndex_ = std::min(currentIndex_, fileIndexList_.size() - 1);
  topIndex_ = 0;
  isDirty_ = true;
}

void SelectProjectView::getSelectedProjectName(char *name) {
  strcpy(name, selection_);
}

void SelectProjectView::getHighlightedProjectName(char *name) {
  if (currentIndex_ >= fileIndexList_.size()) {
    return;
  }

  auto fs = FileSystem::GetInstance();
  unsigned fileIndex = fileIndexList_[currentIndex_];
  fs->getFileName(fileIndex, name, MAX_PROJECT_NAME_LENGTH + 1);
}

void SelectProjectView::SelectButton(int direction) {
  selectedButton_ = (numButtons_ + selectedButton_ + direction) % numButtons_;
  isDirty_ = true;
}

void SelectProjectView::LoadProject() {
  if (currentIndex_ >= fileIndexList_.size()) {
    return;
  }

  // all subdirs directly inside /project are expected to be projects
  unsigned fileIndex = fileIndexList_[currentIndex_];
  auto fs = FileSystem::GetInstance();
  fs->getFileName(fileIndex, selection_, MAX_PROJECT_NAME_LENGTH + 1);
  if (strlen(selection_) == 0) {
    Trace::Log("SELECTPROJECTVIEW",
               "skipping too long project name on Index:%d", fileIndex);
    return;
  }

  Trace::Log("SELECTPROJECTVIEW", "Select Project:%s", selection_);
  // save newly opened projectname, it will be used to load the project file
  // on device boots following the reboot below
  auto ps = PersistencyService::GetInstance();
  ps->SaveProjectState(selection_);

  // now need to delete autosave file so its not loaded when we reboot
  ps->ClearAutosave(selection_);

  // now reboot!
  System *sys = System::GetInstance();
  sys->SystemReboot();
}

bool SelectProjectView::WarnPlayerRunning() {
  if (Player::GetInstance()->IsRunning()) {
    MessageBox *mb = new MessageBox(*this, "Not while running!", MBBF_OK);
    DoModal(mb);
    return true;
  }
  return false;
}

bool SelectProjectView::SelectionIsCurrentProject() {
  char selected[MAX_PROJECT_NAME_LENGTH + 1];
  getHighlightedProjectName(selected);

  auto var = viewData_->project_->FindVariable(FourCC::VarProjectName);
  etl::string<MAX_PROJECT_NAME_LENGTH> projectName = var->GetString();
  const char *current = projectName.c_str();

  return strcmp(current, selected) == 0;
}

bool SelectProjectView::SaveSelectedProject() {
  auto appWindow = static_cast<AppWindow *>(&w_);
  PersistencyService *ps = PersistencyService::GetInstance();

  auto var = viewData_->project_->FindVariable(FourCC::VarProjectName);
  etl::string<MAX_PROJECT_NAME_LENGTH> projectName = var->GetString();
  const char *current = projectName.c_str();

  char selected[MAX_PROJECT_NAME_LENGTH + 1];
  getHighlightedProjectName(selected);

  Trace::Error("%s -> %s", current, selected);

  if (ps->Save(selected, current, true) != PERSIST_SAVED) {
    return false;
  }

  // all good so now persist the new project name in project state
  bool result = ps->SaveProjectState(selected) == PERSIST_SAVED;

  if (result) {
    viewData_->project_->SetProjectName(selected);
  }

  return result;
}

void SelectProjectView::ConfirmOverwrite() {
  char selected[MAX_PROJECT_NAME_LENGTH + 1];
  getHighlightedProjectName(selected);

  char buffer[MAX_PROJECT_NAME_LENGTH + 8];
  snprintf(buffer, sizeof(buffer), "\"%s\"?", selected);

  MessageBox *mb = new MessageBox(*this, "Overwrite existing project", buffer,
                                  MBBF_YES | MBBF_NO);
  DoModal(mb, ConfirmOverwriteCallback);
}

void SelectProjectView::AttemptDeletingSelectedProject() {
  if (WarnPlayerRunning()) {
    return;
  }

  if (SelectionIsCurrentProject()) {
    MessageBox *mb =
        new MessageBox(*this, "Cannot delete the current", "project.", MBBF_OK);
    DoModal(mb);
    return;
  }

  DeleteProject();
}