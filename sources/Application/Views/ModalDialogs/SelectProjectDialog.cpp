
#include "SelectProjectDialog.h"
#include "Application/Views/ModalDialogs/MessageBox.h"
#include "NewProjectDialog.h"
#include "System/Console/Trace.h"

#include <algorithm>

#define LIST_PAGE_SIZE 14
#define LIST_WIDTH 26

static const char *buttonText[2] = {"Load", "New"};

int SelectProjectDialog::lastProject_ = 0;

static void NewProjectCallback(View &v, ModalView &dialog) {

  NewProjectDialog &npd = (NewProjectDialog &)dialog;
  if (dialog.GetReturnCode() > 0) {
    etl::string selected = npd.GetName();
    SelectProjectDialog &spd = (SelectProjectDialog &)v;
    Result result = spd.OnNewProject(selected.c_str());
    if (result.Failed()) {
      Trace::Error(result.GetDescription().c_str());
    }
  }
};

SelectProjectDialog::SelectProjectDialog(View &view) : ModalView(view) {}

SelectProjectDialog::~SelectProjectDialog() {}

void SelectProjectDialog::DrawView() {
  SetColor(CD_NORMAL);

  int offset = LIST_WIDTH / 3;
  // Draw projects
  SetWindow(LIST_WIDTH, LIST_PAGE_SIZE);
  GUITextProperties props;

  auto picoFS = PicoFileSystem::GetInstance();

  int x = 0;
  int y = 0;

  // need to use fullsize buffer as sdfat doesnt truncate if filename longer
  // than buffer but instead returns empty string  in buffer :-(
  char buffer[PFILENAME_SIZE];
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

    if (picoFS->getFileType(fileIndex) != PFT_DIR) {
      picoFS->getFileName(fileIndex, buffer, PFILENAME_SIZE);
    } else {
      picoFS->getFileName(fileIndex, buffer, PFILENAME_SIZE);
    }
    // make sure truncate to list width the filename with trailing null
    buffer[LIST_WIDTH] = 0;
    DrawString(x, y, buffer, props);
    y += 1;
  };

  SetColor(CD_NORMAL);

  for (int i = 0; i < 2; i++) {
    const char *text = buttonText[i];
    x = offset * (i + 1) - strlen(text) / 2;
    props.invert_ = (i == selected_) ? true : false;
    DrawString(x, y, text, props);
  }
};

void SelectProjectDialog::OnPlayerUpdate(PlayerEventType,
                                         unsigned int currentTick){};

void SelectProjectDialog::OnFocus() {
  setCurrentFolder();
  currentProject_ = lastProject_;
};

void SelectProjectDialog::ProcessButtonMask(unsigned short mask, bool pressed) {
  if (!pressed)
    return;

  if (mask & EPBM_B) {
    if (mask & EPBM_UP)
      warpToNextProject(true);
    if (mask & EPBM_DOWN)
      warpToNextProject(false);
  } else {
    // A modifier
    if (mask & EPBM_A) {
      switch (selected_) {
      case 0: // load
      {
        // all subdirs in /project are expected to be projects
        unsigned fileIndex = fileIndexList_[currentIndex_];
        char name[PFILENAME_SIZE];
        auto picoFS = PicoFileSystem::GetInstance();
        picoFS->getFileName(fileIndex, selection_, PFILENAME_SIZE);

        // load the project
        EndModal(1);

        break;
      }
      case 1: // new
      {
        NewProjectDialog *npd = new NewProjectDialog(*this);
        DoModal(npd, NewProjectCallback);
        break;
      }
      }
    } else {
      // R Modifier
      if (mask & EPBM_R) {
      } else {
        // No modifier
        if (mask == EPBM_UP)
          warpToNextProject(true);
        if (mask == EPBM_DOWN)
          warpToNextProject(false);
        if (mask == EPBM_LEFT) {
          selected_--;
          if (selected_ < 0)
            selected_ += 2;
          isDirty_ = true;
        }
        if (mask == EPBM_RIGHT) {
          selected_ = (selected_ + 1) % 2;
          isDirty_ = true;
        }
      }
    }
  }
}

void SelectProjectDialog::warpToNextProject(bool goUp) {

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

Result SelectProjectDialog::OnNewProject(const char *name) {

  Trace::Log("SELECTPROJDIALOG", "creating project:%s", name);

  strcpy(selection_, name);

  etl::string<MAX_PROJECT_NAME_LENGTH + PFILENAME_SIZE + 20> path("/projects/");
  path.append(name);

  bool res = PicoFileSystem::GetInstance()->makeDir(path.c_str());
  auto result = res ? Result::NoError : Result("");
  RETURN_IF_FAILED_MESSAGE(result, "Failed to create project dir");

  path.append("/samples");
  Trace::Log("SELECTPROJDIALOG", "creating samples dir at %s", path.c_str());
  res = PicoFileSystem::GetInstance()->makeDir(path.c_str());
  result = res ? Result::NoError : Result("");
  RETURN_IF_FAILED_MESSAGE(result, "Failed to create samples dir");

  EndModal(1);
  return Result::NoError;
};

// copy-paste-mutilate'd from ImportSampleDialog
void SelectProjectDialog::setCurrentFolder() {
  auto picoFS = PicoFileSystem::GetInstance();
  picoFS->chdir("/projects");

  // get ready
  selected_ = 0;
  fileIndexList_.clear();

  // Let's read all the directory in the project dir
  picoFS->list(&fileIndexList_, NULL, true);

  // temp hack,  filter out "." & ".."
  fileIndexList_.erase(fileIndexList_.begin());
  fileIndexList_.erase(fileIndexList_.begin());

  // reset & redraw screen
  topIndex_ = 0;
  currentProject_ = 0;
  isDirty_ = true;
}
