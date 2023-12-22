
#include "SelectProjectDialog.h"
#include "Application/Views/ModalDialogs/MessageBox.h"
#include "NewProjectDialog.h"
#include "System/Console/Trace.h"

#include <algorithm>

#define LIST_SIZE 14
#define LIST_WIDTH 26

static const char *buttonText[2] = {"Load", "New"};

Path SelectProjectDialog::lastFolder_("root:");
int SelectProjectDialog::lastProject_ = 0;

static void NewProjectCallback(View &v, ModalView &dialog) {

  NewProjectDialog &npd = (NewProjectDialog &)dialog;
  if (dialog.GetReturnCode() > 0) {
    std::string selected = npd.GetName();
    SelectProjectDialog &spd = (SelectProjectDialog &)v;
    Result result = spd.OnNewProject(selected);
    if (result.Failed()) {
      Trace::Error(result.GetDescription().c_str());
    }
  }
};

SelectProjectDialog::SelectProjectDialog(View &view)
    : ModalView(view), content_(true) {}

SelectProjectDialog::~SelectProjectDialog() {}

void SelectProjectDialog::DrawView() {

  SetWindow(LIST_WIDTH, LIST_SIZE + 3);

  GUITextProperties props;

  SetColor(CD_NORMAL);

  // Draw projects

  int x = 1;
  int y = 1;

  if (currentProject_ < topIndex_) {
    topIndex_ = currentProject_;
  };
  if (currentProject_ >= topIndex_ + LIST_SIZE) {
    topIndex_ = currentProject_ - LIST_SIZE + 1;
  };

  int count = 0;
  char buffer[256];
  for (content_.Begin(); !content_.IsDone(); content_.Next()) {
    if ((count >= topIndex_) && (count < topIndex_ + LIST_SIZE)) {
      Path &current = content_.CurrentItem();
      std::string p = current.GetName();

      std::string firstFourChars = p.substr(0, 4);
      std::transform(firstFourChars.begin(), firstFourChars.end(),
                     firstFourChars.begin(), ::tolower);
      if (firstFourChars == "lgpt" && p.size() > 4) {
        int namestart = 4;
        // skip _ if needed
        if ((!isalnum(p[4])) && (p.size() > 4)) {
          namestart++;
        }
        std::string t = p;
        p = " ";
        p += t.substr(namestart);
      } else {
        std::string t = p;
        p = "[";
        p += t;
        p += "]";
      };

      if (count == currentProject_) {
        SetColor(CD_HILITE2);
        props.invert_ = true;
      } else {
        SetColor(CD_NORMAL);
        props.invert_ = false;
      }
      strcpy(buffer, p.c_str());
      buffer[LIST_WIDTH - 1] = 0;
      DrawString(x, y, buffer, props);
      y += 1;
    }
    count++;
  };

  y = LIST_SIZE + 2;
#ifndef NO_EXIT
  int offset = LIST_WIDTH / 4;
#else
  int offset = LIST_WIDTH / 3;
#endif

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

  setCurrentFolder(lastFolder_);
  currentProject_ = lastProject_;
};

void SelectProjectDialog::ProcessButtonMask(unsigned short mask, bool pressed) {
  if (!pressed)
    return;

  if (mask & EPBM_B) {
    if (mask & EPBM_UP)
      warpToNextProject(-LIST_SIZE);
    if (mask & EPBM_DOWN)
      warpToNextProject(LIST_SIZE);
  } else {

    // A modifier
    if (mask & EPBM_A) {
      switch (selected_) {
      case 0: // load
      {
        // locate folder user had selected when they hit a
        int count = 0;
        Path *current = 0;

        for (content_.Begin(); !content_.IsDone(); content_.Next()) {
          if (count == currentProject_) {
            current = &content_.CurrentItem();
            break;
          }
          count++;
        }

        // check if folder is a project, indicated by 'lgpt' being the first 4
        // characters of the folder name
        std::string name = current->GetName();
        std::string firstFourChars = name.substr(0, 4);
        std::transform(firstFourChars.begin(), firstFourChars.end(),
                       firstFourChars.begin(), ::tolower);
        if (firstFourChars == "lgpt") {
          // ugly hack to make the "name" include subdirectories
          // we pass along everything past the root dir
          selection_ = *current;
          lastFolder_ = currentPath_;
          lastProject_ = currentProject_;
          // load the project
          EndModal(1);
        } else {
          if (current->GetName() == "..") {
            Path parent = currentPath_.GetParent();
            setCurrentFolder(parent);
          } else {
            Path newdir = *current;
            setCurrentFolder(newdir);
          }
        }
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
          warpToNextProject(-1);
        if (mask == EPBM_DOWN)
          warpToNextProject(1);
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

void SelectProjectDialog::warpToNextProject(int amount) {

  int offset = currentProject_ - topIndex_;
  int size = content_.Size();
  currentProject_ += amount;
  if (currentProject_ < 0)
    currentProject_ += size;
  if (currentProject_ >= size)
    currentProject_ -= size;

  if ((amount > 1) || (amount < -1)) {
    topIndex_ = currentProject_ - offset;
    if (topIndex_ < 0) {
      topIndex_ = 0;
    };
  }
  isDirty_ = true;
}

Path SelectProjectDialog::GetSelection() { return selection_; }

Result SelectProjectDialog::OnNewProject(std::string &name) {
  Path path = currentPath_.Descend(name);
  Trace::Log("TMP", "creating project at %s", path.GetPath().c_str());
  selection_ = path;
  Result result = FileSystem::GetInstance()->MakeDir(path.GetPath().c_str());
  std::string buf("Failed to create project dir for ");
  buf.append(path.GetPath().c_str());
  RETURN_IF_FAILED_MESSAGE(result, buf);

  path = path.Descend("samples");
  Trace::Log("TMP", "creating samples dir at %s", path.GetPath().c_str());
  result = FileSystem::GetInstance()->MakeDir(path.GetPath().c_str());
  buf = "Failed to create samples dir for ";
  buf.append(path.GetPath().c_str());
  RETURN_IF_FAILED_MESSAGE(result, buf);

  EndModal(1);
  return Result::NoError;
};

// copy-paste-mutilate'd from ImportSampleDialog
void SelectProjectDialog::setCurrentFolder(Path &path) {

  // get ready
  selected_ = 0;
  currentPath_ = path;
  content_.Empty();

  // Let's read all the directory in the root

  I_Dir *dir = FileSystem::GetInstance()->Open(currentPath_.GetPath().c_str());

  if (dir) {
    // Get all lgpt something
    dir->GetContent("*");
    dir->Sort();
    for (dir->Begin(); !dir->IsDone(); dir->Next()) {
      Path &path = dir->CurrentItem();
      if (path.IsDirectory()) {
        std::string name = path.GetName();
        if (name[0] != '.' || name[1] == '.') {
          Path *p = new Path(path);
          content_.Insert(p);
        }
      }
    }
    delete (dir);
  }
  // reset & redraw screen
  topIndex_ = 0;
  currentProject_ = 0;
  isDirty_ = true;
}
