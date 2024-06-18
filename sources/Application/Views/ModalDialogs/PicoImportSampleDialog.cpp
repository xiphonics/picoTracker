#include "PicoImportSampleDialog.h"
#include "Application/Instruments/SampleInstrument.h"
#include "Application/Instruments/SamplePool.h"
#include "pico/multicore.h"
#include <memory>

#define LIST_WIDTH 24
#define SAMPLE_LIB "/samplelib"

PicoImportSampleDialog::PicoImportSampleDialog(View &view) : ModalView(view) {
  Trace::Log("PICOSAMPLEIMPORT", "samplelib is:%s", SAMPLE_LIB);
  fileIndexList_.fill(0);
}

PicoImportSampleDialog::~PicoImportSampleDialog() {
  Trace::Log("PICOSAMPLEIMPORT", "Destruct ===");
}

void PicoImportSampleDialog::DrawView() {
  Trace::Log("PICOSAMPLEIMPORT", "DrawView current:%d topIdx:%d", currentIndex_,
             topIndex_);

  SetWindow(LIST_WIDTH, PAGED_PAGE_SIZE);
  GUITextProperties props;

  auto picoFS = PicoFileSystem::GetInstance();

  picoFS->list(&fileIndexList_);

  // Draw samples
  int x = 0;
  int y = 0;

  // need to use fullsize buffer as sdfat doesnt truncate if filename longer
  // than buffer but instead returns empty string  in buffer :-(
  char buffer[PFILENAME_SIZE];
  for (size_t i = topIndex_;
       i < topIndex_ + PAGED_PAGE_SIZE && (i < fileIndexList_.size()); i++) {
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
      buffer[0] = '[';
      picoFS->getFileName(fileIndex, buffer + 1, PFILENAME_SIZE);
      buffer[LIST_WIDTH - 2] = ']';
    }
    // make sure truncate to list width the filename with trailing null
    buffer[LIST_WIDTH] = 0;
    DrawString(x, y, buffer, props);
    y += 1;
  };

  SetColor(CD_NORMAL);
};

void PicoImportSampleDialog::warpToNextSample(int direction) {
  currentIndex_ += direction;

  if (currentIndex_ < 0) {
    topIndex_ = currentIndex_ = 0;
  }
  if (currentIndex_ >= fileIndexList_.size()) {
    currentIndex_ = fileIndexList_.size() - 1;
    topIndex_ = currentIndex_ - PAGED_PAGE_SIZE;
  }

  // if we have scrolled off the bottom, page the file list down if not at end
  // of the list
  if ((currentIndex_ >= (topIndex_ + PAGED_PAGE_SIZE)) &&
      ((topIndex_ + PAGED_PAGE_SIZE) < fileIndexList_.size())) {
    topIndex_++;
  }

  // if we have scrolled off the top, page the file list up if not already at
  // very top of the list
  if (currentIndex_ < topIndex_) {
    topIndex_ = currentIndex_;
  }
  isDirty_ = true;
}

void PicoImportSampleDialog::OnPlayerUpdate(PlayerEventType,
                                            unsigned int currentTick) {};

void PicoImportSampleDialog::OnFocus() {
  auto picoFS = PicoFileSystem::GetInstance();

  toInstr_ = viewData_->currentInstrument_;

  if (!picoFS->chdir(SAMPLE_LIB)) {
    Trace::Error("FAILED to chdir to /samplelib");
  } else {
    Trace::Log("PICOIMPORTSAMPLEDIALOG", "chdir to /samplelib");
  }
};

void PicoImportSampleDialog::preview(Path &element) {
  // if (Player::GetInstance()->IsPlaying()) {
  //   Player::GetInstance()->StopStreaming();
  //   if (currentSample_ != previewPlayingIndex_) {
  //     previewPlayingIndex_ = currentSample_;
  //     Player::GetInstance()->StartStreaming(element);
  //   }
  // } else {
  //   Player::GetInstance()->StartStreaming(element);
  //   previewPlayingIndex_ = currentSample_;
  // }
}

void PicoImportSampleDialog::import(Path &element) {

  //   SamplePool *pool = SamplePool::GetInstance();

  // #ifdef PICOBUILD
  //   // Pause core1 in order to be able to write to flash and ensure core1 is
  //   // not reading from it, it also disables IRQs on it
  //   //
  //   https://www.raspberrypi.com/documentation/pico-sdk/high_level.html#multicore_lockout
  //   multicore_lockout_start_blocking();
  // #endif
  //   int sampleID = pool->ImportSample(element);
  // #ifdef PICOBUILD
  //   multicore_lockout_end_blocking();
  // #endif

  //   if (sampleID >= 0) {
  //     I_Instrument *instr =
  //         viewData_->project_->GetInstrumentBank()->GetInstrument(toInstr_);
  //     if (instr->GetType() == IT_SAMPLE) {
  //       SampleInstrument *sinstr = (SampleInstrument *)instr;
  //       sinstr->AssignSample(sampleID);
  //       toInstr_ = viewData_->project_->GetInstrumentBank()->GetNext();
  //     };
  //   } else {
  //     Trace::Error("failed to import sample");
  //   };
  //   isDirty_ = true;
};

void PicoImportSampleDialog::ProcessButtonMask(unsigned short mask,
                                               bool pressed) {
  if (!pressed)
    return;

  if (mask & EPBM_START) {
    if (mask & EPBM_L) {
      Trace::Log("PICOIMPORT", "SHIFT play - import");
      // import(fullPath);
    } else {
      Trace::Log("PICOIMPORT", "plain play preview");
      // preview(fullPath);
    }
    // handle moving up and down the file list
  } else if (mask & EPBM_UP) {
    warpToNextSample(-1);
    printf("up");
  } else if (mask & EPBM_DOWN) {
    warpToNextSample(1);
    printf("down");
  } else if ((mask & EPBM_LEFT) && (mask & EPBM_R)) {
    EndModal(0);
  } else {
    // A modifier
    if (mask & EPBM_A) {
      auto picoFS = PicoFileSystem::GetInstance();
      unsigned fileIndex = fileIndexList_[currentIndex_];
      char name[PFILENAME_SIZE];
      picoFS->getFileName(fileIndex, name, PFILENAME_SIZE);
      if (picoFS->getFileType(fileIndex) == PFT_DIR) {
        picoFS->chdir(name);

        // if (currentItem.name == std::string("..")) {
        //   if (currentPath_.GetPath() == std::string(SAMPLE_LIB_PATH)) {
        //     Trace::Log("PAGEDIMPORT",
        //                "already at root of samplelib nothing to do");
        //   } else {
        //     Path parent = currentPath_.GetParent();
        //     setCurrentFolder(&parent);
        //   }
        // } else {
        //   auto fullName = currentDir_->getFullName(currentItem.index);
        //   Path childDirPath = currentPath_.Descend(fullName);
        //   setCurrentFolder(&childDirPath);
        // }
        isDirty_ = true;
        return;
      }
    }
  }
}

void PicoImportSampleDialog::setCurrentFolder(Path *path) {
  //   Trace::Log("PAGEDIMPORT", "set Current Folder:%s",
  //   path->GetPath().c_str()); Path formerPath(currentPath_); topIndex_ = 0;
  //   currentSample_ = 0;
  //   currentPath_ = Path(*path);
  //   fileList_.clear();
  //   if (path) {
  //     // make sure we free mem from existing currentDir instance
  //     if (currentDir_ != NULL) {
  //       delete currentDir_;
  //     }
  //     currentDir_ =
  //     FileSystem::GetInstance()->OpenPaged(path->GetPath().c_str());
  //     // TODO: show "Loading..." mesg in UI
  //     Trace::Log("PAGEDIMPORT", "Loading...");
  //     currentDir_->GetContent("*.wav");
  //     // TODO: hide "Loading..." mesg in UI
  //     Trace::Log("PAGEDIMPORT", "Finished Loading");
  //   }

  //   // load first page of file/subdirs
  //   fileList_.clear();
  //   currentDir_->getFileList(topIndex_, &fileList_);
}
