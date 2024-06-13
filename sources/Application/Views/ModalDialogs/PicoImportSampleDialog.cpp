#include "PicoImportSampleDialog.h"
#include "Application/Instruments/SampleInstrument.h"
#include "Application/Instruments/SamplePool.h"
#include "pico/multicore.h"
#include <memory>

#define LIST_WIDTH 24
#define SAMPLE_LIB "samplelib"

PicoImportSampleDialog::PicoImportSampleDialog(View &view) : ModalView(view) {
  Trace::Log("PICOSAMPLEIMPORT", "samplelib is:%s", SAMPLE_LIB);
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

  // Draw samples
  int x = 0;
  int y = 0;

  int count = 0;
  char buffer[LIST_WIDTH + 1];
  for (const int &current : fileIndexList_) {
    if (count == (currentIndex_ % PAGED_PAGE_SIZE)) {
      SetColor(CD_HILITE2);
      props.invert_ = true;
    } else {
      SetColor(CD_NORMAL);
      props.invert_ = false;
    }
    if (picoFS->getFileType(current) != PFT_DIR) {
      picoFS->getFileName(current, buffer, LIST_WIDTH);
    } else {
      buffer[0] = '[';
      picoFS->getFileName(current, buffer, LIST_WIDTH - 2);
      strcat(buffer, "]");
    }
    // make sure if truncated the filename we have trailing null
    buffer[LIST_WIDTH] = 0;
    DrawString(x, y, buffer, props);
    y += 1;
    count++;
  };

  SetColor(CD_NORMAL);
};

void PicoImportSampleDialog::warpToNextSample(int direction) {
  // currentSample_ += direction;
  // int size = currentDir_->size();
  // bool needPage = false;

  // // wrap around from first entry to last entry
  // if (currentSample_ < 0) {
  //   topIndex_ = (size / PAGED_PAGE_SIZE) * PAGED_PAGE_SIZE;
  //   currentSample_ = size - 1; // goto last entry
  //   needPage = true;
  // }
  // // wrap around from last entry to first entry
  // if (currentSample_ >= size) {
  //   currentSample_ = 0;
  //   topIndex_ = 0;
  //   needPage = true;
  // }

  // // if we have scrolled off the bottom, page the file list down if not at
  // end
  // // of the list
  // if ((currentSample_ >= (topIndex_ + PAGED_PAGE_SIZE)) &&
  //     ((topIndex_ + PAGED_PAGE_SIZE) < size)) {
  //   topIndex_ += PAGED_PAGE_SIZE;
  //   needPage = true;
  // }

  // // if we have scrolled off the top, page the file list up if not already at
  // // very top of the list
  // if (currentSample_ < topIndex_ && topIndex_ != 0) {
  //   topIndex_ -= PAGED_PAGE_SIZE;
  //   needPage = true;
  // }

  // // need to fetch a new page of the file list of current directory
  // if (needPage) {
  //   fileList_.clear();
  //   currentDir_->getFileList(topIndex_, &fileList_);
  // }

  // isDirty_ = true;
}

void PicoImportSampleDialog::OnPlayerUpdate(PlayerEventType,
                                            unsigned int currentTick) {};

void PicoImportSampleDialog::OnFocus() {
  auto picoFS = PicoFileSystem::GetInstance();
  picoFS->chRootDir();
  picoFS->chdir(SAMPLE_LIB);
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

  //   if (!pressed)
  //     return;
  //   // make sure to index into the fileList with the offset from topIndex!
  //   FileListItem currentItem = fileList_[currentSample_ - topIndex_];

  //   auto fullPathStr = std::string(currentPath_.GetPath());
  //   fullPathStr += "/";
  //   fullPathStr += currentItem.name;
  //   auto fullPath = Path{fullPathStr};

  //   if (mask & EPBM_START) {
  //     if (mask & EPBM_L) {
  //       Trace::Log("PAGEDIMPORT", "SHIFT play - import");
  //       import(fullPath);
  //     } else {
  //       Trace::Log("PAGEDIMPORT", "plain play preview");
  //       preview(fullPath);
  //     }
  //   } else if ((mask & EPBM_LEFT) && (mask & EPBM_R)) {
  //     // make sure we free mem from existing currentDir instance
  //     if (currentDir_ != NULL) {
  //       delete currentDir_;
  //     }
  //     EndModal(0);
  //   } else if (mask & EPBM_B) {
  //     if (mask & EPBM_UP)
  //       warpToNextSample(-PAGED_PAGE_SIZE);
  //     if (mask & EPBM_DOWN)
  //       warpToNextSample(PAGED_PAGE_SIZE);
  //   } else {
  //     // A modifier
  //     if (mask & EPBM_A) {
  //       if (currentItem.isDirectory) {
  //         if (currentItem.name == std::string("..")) {
  //           if (currentPath_.GetPath() == std::string(SAMPLE_LIB_PATH)) {
  //             Trace::Log("PAGEDIMPORT",
  //                        "already at root of samplelib nothing to do");
  //           } else {
  //             Path parent = currentPath_.GetParent();
  //             setCurrentFolder(&parent);
  //           }
  //         } else {
  //           auto fullName = currentDir_->getFullName(currentItem.index);
  //           Path childDirPath = currentPath_.Descend(fullName);
  //           setCurrentFolder(&childDirPath);
  //         }
  //         isDirty_ = true;
  //         return;
  //       }
  //     }
  //     // handle moving up and down the file list
  //     if (mask & EPBM_UP) {
  //       warpToNextSample(-1);
  //     } else if (mask & EPBM_DOWN) {
  //       warpToNextSample(1);
  //     }
  //   }
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
