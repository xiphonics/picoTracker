#include "PagedImportSampleDialog.h"
#include "Application/Instruments/SampleInstrument.h"
#include "Application/Instruments/SamplePool.h"
#include "pico/multicore.h"
#include <memory>

#define LIST_SIZE 15
#define LIST_WIDTH 24

static const char *buttonText[3] = {"Listen", "Import", "Exit"};

PagedImportSampleDialog::PagedImportSampleDialog(View &view) : ModalView(view) {
  selected_ = 0;
  fileList_.reserve(15);
  Trace::Log("PAGEDIMPORT", "samplelib is:%s", SAMPLE_LIB_PATH);
}

PagedImportSampleDialog::~PagedImportSampleDialog() {
  Trace::Log("PAGEDIMPORT", "Destruct ===");
}

void PagedImportSampleDialog::DrawView() {
  Trace::Log("PAGEDIMPORT", "DrawView current:%d topIdx:%d", currentSample_,
             topIndex_);

  SetWindow(LIST_WIDTH, LIST_SIZE + 3);
  GUITextProperties props;

// Draw title
#ifdef SHOW_MEM_USAGE
  char title[40];

  SetColor(CD_NORMAL);

  sprintf(title, "MEM [%d]", System::GetInstance()->GetMemoryUsage());
  GUIPoint pos = GUIPoint(0, 0);
  w_.DrawString(title, pos, props);

#endif
  // Draw samples
  int x = 1;
  int y = 1;

  int count = 0;
  char buffer[LIST_WIDTH + 1];
  for (const FileListItem &current : fileList_) {
    if (count == (currentSample_ % LIST_SIZE)) {
      SetColor(CD_HILITE2);
      props.invert_ = true;
    } else {
      SetColor(CD_NORMAL);
      props.invert_ = false;
    }
    if (!current.isDirectory) {
      strncpy(buffer, current.name.c_str(), LIST_WIDTH);
    } else {
      buffer[0] = '[';
      strncpy(buffer + 1, current.name.c_str(), LIST_WIDTH - 2);
      strcat(buffer, "]");
    }
    buffer[LIST_WIDTH] =
        0; // make sure if truncated the filename we have trailing null
    DrawString(x, y, buffer, props);
    y += 1;
    count++;
  };

  y = LIST_SIZE + 2;
  int offset = 7;

  SetColor(CD_NORMAL);

  // Draw buttons
  for (int i = 0; i < 3; i++) {
    const char *text = buttonText[i];
    x = (offset * (i + 1) - strlen(text) / 2) - 2;
    props.invert_ = (i == selected_) ? true : false;
    DrawString(x, y, text, props);
  }
};

void PagedImportSampleDialog::warpToNextSample(int direction) {
  currentSample_ += direction;
  int size = currentDir_->size();
  bool needPage = false;

  // wrap around from first entry to last entry
  if (currentSample_ < 0) {
    topIndex_ = (size / LIST_SIZE) * LIST_SIZE;
    currentSample_ = size - 1; // goto last entry
    needPage = true;
  }
  // wrap around from last entry to first entry
  if (currentSample_ >= size) {
    currentSample_ = 0;
    topIndex_ = 0;
    needPage = true;
  }

  // if we have scrolled off the bottom, page the file list down if not at end
  // of the list
  if ((currentSample_ >= (topIndex_ + LIST_SIZE)) &&
      ((topIndex_ + LIST_SIZE) < size)) {
    topIndex_ += LIST_SIZE;
    needPage = true;
  }

  // if we have scrolled off the top, page the file list up if not already at
  // very top of the list
  if (currentSample_ < topIndex_ && topIndex_ != 0) {
    topIndex_ -= LIST_SIZE;
    needPage = true;
  }

  // need to fetch a new page of the file list of current directory
  if (needPage) {
    fileList_.clear();
    currentDir_->getFileList(topIndex_, &fileList_);
  }

  isDirty_ = true;
}

void PagedImportSampleDialog::OnPlayerUpdate(PlayerEventType,
                                             unsigned int currentTick){};

void PagedImportSampleDialog::OnFocus() {
  Path current(currentPath_);
  setCurrentFolder(&current);
  toInstr_ = viewData_->currentInstrument_;
};

void PagedImportSampleDialog::preview(Path &element) {
  Player::GetInstance()->StartStreaming(element);
}

void PagedImportSampleDialog::import(Path &element) {

  SamplePool *pool = SamplePool::GetInstance();

#ifdef PICO_BUILD
  // Pause core1 in order to be able to write to flash and ensure core1 is
  // not reading from it, it also disables IRQs on it
  // https://www.raspberrypi.com/documentation/pico-sdk/high_level.html#multicore_lockout
  multicore_lockout_start_blocking();
#endif
  int sampleID = pool->ImportSample(element);
#ifdef PICO_BUILD
  multicore_lockout_end_blocking();
#endif

  if (sampleID >= 0) {
    I_Instrument *instr =
        viewData_->project_->GetInstrumentBank()->GetInstrument(toInstr_);
    if (instr->GetType() == IT_SAMPLE) {
      SampleInstrument *sinstr = (SampleInstrument *)instr;
      sinstr->AssignSample(sampleID);
      toInstr_ = viewData_->project_->GetInstrumentBank()->GetNext();
    };
  } else {
    Trace::Error("failed to import sample");
  };
  isDirty_ = true;
};

void PagedImportSampleDialog::ProcessButtonMask(unsigned short mask,
                                                bool pressed) {

  if (!pressed)
    return;

  if (mask & EPBM_B) {
    if (mask & EPBM_UP)
      warpToNextSample(-LIST_SIZE);
    if (mask & EPBM_DOWN)
      warpToNextSample(LIST_SIZE);
  } else {
    // A modifier
    if (mask & EPBM_A) {
      // make sure to index into the fileList with the offset from topIndex!
      FileListItem currentItem = fileList_[currentSample_ - topIndex_];

      if ((selected_ != 2) && currentItem.isDirectory) {
        if (currentItem.name == std::string("..")) {
          if (currentPath_.GetPath() == std::string(SAMPLE_LIB_PATH)) {
            Trace::Log("PAGEDIMPORT",
                       "already at root of samplelib nothing to do");
          } else {
            Path parent = currentPath_.GetParent();
            setCurrentFolder(&parent);
          }
        } else {
          auto fullName = currentDir_->getFullName(currentItem.index);
          Path childDirPath = currentPath_.Descend(fullName);
          setCurrentFolder(&childDirPath);
        }
        isDirty_ = true;
        return;
      }
      auto fullPathStr = std::string(currentPath_.GetPath());
      fullPathStr += "/";
      fullPathStr += currentItem.name;
      auto fullPath = Path{fullPathStr};

      switch (selected_) {
      case 0: // preview
        preview(fullPath);
        break;
      case 1: // import
        import(fullPath);
        break;
      case 2: // Exit ;
        // make sure we free mem from existing currentDir instance
        if (currentDir_ != NULL) {
          delete currentDir_;
        }
        EndModal(0);
        break;
      }
    } else {
      // R Modifier
      if (mask & EPBM_R) {
      } else {
        // No modifier
        if (mask == EPBM_UP)
          warpToNextSample(-1);
        if (mask == EPBM_DOWN)
          warpToNextSample(1);
        if (mask == EPBM_LEFT) {
          selected_ -= 1;
          if (selected_ < 0)
            selected_ += 3;
          isDirty_ = true;
        }
        if (mask == EPBM_RIGHT) {
          selected_ = (selected_ + 1) % 3;
          isDirty_ = true;
        }
      }
    }
  }
}

void PagedImportSampleDialog::setCurrentFolder(Path *path) {
  Trace::Log("PAGEDIMPORT", "set Current Folder:%s", path->GetPath().c_str());
  Path formerPath(currentPath_);
  topIndex_ = 0;
  currentSample_ = 0;
  currentPath_ = Path(*path);
  fileList_.clear();
  if (path) {
    // make sure we free mem from existing currentDir instance
    if (currentDir_ != NULL) {
      delete currentDir_;
    }
    currentDir_ = FileSystem::GetInstance()->OpenPaged(path->GetPath().c_str());
    // TODO: show "Loading..." mesg in UI
    Trace::Log("PAGEDIMPORT", "Loading...");
    currentDir_->GetContent("*.wav");
    // TODO: hide "Loading..." mesg in UI
    Trace::Log("PAGEDIMPORT", "Finished Loading");
  }

  // load first page of file/subdirs
  fileList_.clear();
  currentDir_->getFileList(topIndex_, &fileList_);
}
