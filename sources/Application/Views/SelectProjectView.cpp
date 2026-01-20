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
#include "BaseClasses/ViewEvent.h"

#define LIST_PAGE_SIZE SCREEN_HEIGHT - 2
#define LIST_WIDTH 26
#define INVALID_PROJECT_NAME "INVALID NAME"

SelectProjectView::SelectProjectView(GUIWindow &w, ViewData *viewData)
    : ScreenView(w, viewData) {}

SelectProjectView::~SelectProjectView() {}

void SelectProjectView::Reset() {
  topIndex_ = 0;
  currentIndex_ = 0;
  selection_[0] = '\0';
  fileIndexList_.clear();
}

void SelectProjectView::DrawView() {
  Clear();

  GUITextProperties props;
  GUIPoint pos = GetTitlePosition();
  SetColor(CD_NORMAL);

  auto fs = FileSystem::GetInstance();

  // Draw title
  const char *title = "Load Project";
  SetColor(CD_INFO);
  DrawString(pos._x + 1, pos._y, title, props);

  SetColor(CD_NORMAL);

  // Draw projects
  int x = 1;
  int y = pos._y + 2;

  char buffer[MAX_PROJECT_NAME_LENGTH + 1];
  for (size_t i = topIndex_;
       i < topIndex_ + LIST_PAGE_SIZE && (i < fileIndexList_.size()); i++) {
    if (i == currentIndex_) {
      SetColor(CD_HILITE2);
      props.invert_ = true;
    } else {
      SetColor(CD_NORMAL);
      props.invert_ = false;
    }

    memset(buffer, '\0', sizeof(buffer));
    unsigned fileIndex = fileIndexList_[i];

    if (fs->getFileType(fileIndex) == PFT_DIR) {
      fs->getFileName(fileIndex, buffer, MAX_PROJECT_NAME_LENGTH + 1);
    }
    // SDFat lib doesnt truncate if filename longer than buffer as per docs but
    // instead returns empty string in buffer
    if (strlen(buffer) == 0) {
      strcpy(buffer, INVALID_PROJECT_NAME);
    }
    DrawString(x, y, buffer, props);
    y += 1;
  };

  SetColor(CD_NORMAL);
};

void SelectProjectView::OnPlayerUpdate(PlayerEventType,
                                       unsigned int currentTick){};

void SelectProjectView::OnFocus() { setCurrentFolder(); };

void SelectProjectView::ProcessButtonMask(unsigned short mask, bool pressed) {
  if (!pressed)
    return;

  if (mask & EPBM_EDIT) {
    if (mask & EPBM_UP)
      warpToNextProject(true);
    if (mask & EPBM_DOWN)
      warpToNextProject(false);
  } else {
    // A modifier
    if (mask & EPBM_ENTER) {
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

      ViewEvent ve(VET_LOAD_PROJECT, selection_);
      SetChanged();
      NotifyObservers(&ve);
      return;
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

  // Filter out "." and ".." along with the hidden default project entry
  for (auto it = fileIndexList_.begin(); it != fileIndexList_.end();) {
    fs->getFileName(*it, selection_, MAX_PROJECT_NAME_LENGTH + 1);

    const bool isDotEntry =
        (strcmp(selection_, ".") == 0) || (strcmp(selection_, "..") == 0);
    const bool isUntitled = (strcmp(selection_, UNNAMED_PROJECT_NAME) == 0);

    if (isDotEntry || isUntitled) {
      if (isUntitled) {
        Trace::Log("SELECTPROJECTVIEW", "skipping untitled project on Index:%d",
                   static_cast<int>(it - fileIndexList_.begin()));
      }
      it = fileIndexList_.erase(it);
    } else {
      ++it;
    }
  }

  // reset & redraw screen
  topIndex_ = 0;
  currentIndex_ = 0;
  isDirty_ = true;
}

void SelectProjectView::getSelectedProjectName(char *name) {
  strcpy(name, selection_);
}
