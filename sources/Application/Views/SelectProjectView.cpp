#include "SelectProjectView.h"
#include "Application/AppWindow.h"

#include "hardware/watchdog.h"
#include "pico/bootrom.h"

#define LIST_PAGE_SIZE 14
#define LIST_WIDTH 26
#define INVALID_PROJECT_NAME "INVALID NAME"

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
  const char *title = "Load Project";
  SetColor(CD_INFO);
  DrawString(pos._x + 1, pos._y, title, props);

  SetColor(CD_NORMAL);

  drawBattery(props);

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

      Trace::Log("SELECTPROJECTVIEW", "Select Project:%s \n", selection_);
      // save newly opened projectname, it will be used to load the project file
      // on device boots following the reboot below
      auto ps = PersistencyService::GetInstance();
      ps->SaveProjectState(selection_);

      // now need to delete autosave file so its not loaded when we reboot
      ps->ClearAutosave(selection_);

      // now reboot!
      watchdog_reboot(0, 0, 0);
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
  fs->list(&fileIndexList_, NULL, true);

  // temp hack,  filter out "." & ".."
  fileIndexList_.erase(fileIndexList_.begin());
  fileIndexList_.erase(fileIndexList_.begin());

  // filter out the "untitled" project entry
  for (size_t i = 0; i < fileIndexList_.size(); i++) {
    fs->getFileName(fileIndexList_[i], selection_,
                        MAX_PROJECT_NAME_LENGTH + 1);
    if (strcmp(selection_, UNNAMED_PROJECT_NAME) == 0) {
      Trace::Log("SELECTPROJECTVIEW", "skipping untitled project on Index:%d",
                 i);
      fileIndexList_.erase(fileIndexList_.begin() + i);
      break;
    }
  }

  // reset & redraw screen
  topIndex_ = 0;
  isDirty_ = true;
}

void SelectProjectView::getSelectedProjectName(char *name) {
  strcpy(name, selection_);
}
